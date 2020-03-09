#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "process.h"

#define JMP_TO_LOG_PAYLOAD_SIZE ORIGINAL_INSTRUCTIONS_SIZE
#define LOG_AND_JMP_BACK_PAYLOAD_SIZE 36

void init_console() {
  AllocConsole();
  freopen("CONIN$", "r", stdin);
  freopen("CONOUT$", "w", stdout);
  freopen("CONOUT$", "w", stderr);
  printf("DLL loaded!\n");
}

__declspec(dllexport) void dump(int size, const unsigned char *buf) {
  int i;
  for (i = 0; i < size; ++i) {
    printf("%02x ", buf[i]);
  }
  printf("\n\n");
}

uint8_t *build_log_and_jmp_back_payload(uint32_t dump_addr, uint32_t ret_addr) {
  uint8_t *payload = malloc(LOG_AND_JMP_BACK_PAYLOAD_SIZE);
  uint32_t index = 0;
  // save stack, log, then restore stack
  /* pusha */
  payload[index++] = 0x60;
  /* pushf */
  payload[index++] = 0x9C;
  /* push DWORD PTR [esp+0x28] (arg1) */
  payload[index++] = 0xFF,
  payload[index++] = 0x74;
  payload[index++] = 0x24;
  payload[index++] = ARG_1_STACK_OFFSET + 0x20;
  /* push DWORD PTR [esp+0x30] (arg2) */
  payload[index++] = 0xFF;
  payload[index++] = 0x74;
  payload[index++] = 0x24;
  payload[index++] = ARG_2_STACK_OFFSET + 0x20 + sizeof(uint32_t);
  /* mov eax, dump_addr */
  payload[index++] = 0xB8;
  payload[index++] = dump_addr & 0xFF;
  payload[index++] = (dump_addr >> 8) & 0xFF;
  payload[index++] = (dump_addr >> 16) & 0xFF;
  payload[index++] = (dump_addr >> 24) & 0xFF;
  /* call eax */
  payload[index++] = 0xFF;
  payload[index++] = 0xD0;
  /* add esp, 0x08 */
  payload[index++] = 0x83;
  payload[index++] = 0xC4;
  payload[index++] = NUM_ARGS * sizeof(uint32_t);
  /* popf */
  payload[index++] = 0x9D;
  /* popd */
  payload[index++] = 0x61;
  /* original payload */
  for (int i = 0; i < ORIGINAL_INSTRUCTIONS_SIZE; ++i) {
    payload[index++] = original_instructions[i];
  }
  /* jmp ret_addr */
  payload[index++] = 0xE9;
  payload[index++] = ret_addr & 0xFF;
  payload[index++] = (ret_addr >> 8) & 0xFF;
  payload[index++] = (ret_addr >> 16) & 0xFF;
  payload[index++] = (ret_addr >> 24) & 0xFF;
  return payload;
}

uint8_t *build_jmp_to_log_payload(uint32_t code_cave_addr) {
  uint8_t *payload = malloc(ORIGINAL_INSTRUCTIONS_SIZE);
  uint32_t index = 0;
  /* jmp code_cave_addr */
  payload[index++] = 0xE9;
  payload[index++] = code_cave_addr & 0xFF;
  payload[index++] = (code_cave_addr >> 8) & 0xFF;
  payload[index++] = (code_cave_addr >> 16) & 0xFF;
  payload[index++] = (code_cave_addr >> 24) & 0xFF;
  /* nop padding */
  for (int i = index + 1; i <= ORIGINAL_INSTRUCTIONS_SIZE; ++i) {
    payload[index++] = 0x90;
  }
  return payload;
}

uint32_t calculate_relative_address(uint32_t from, uint32_t to) {
  return from - to - 0x05;
}

DWORD write_payload() {
  HANDLE hProcess;
  LPVOID code_cave;
  LPVOID dump_addr;
  uint8_t *jmp_to_log_payload;
  uint8_t *log_and_jmp_back_payload;
  // get handle
  hProcess = get_handle();
  if (!hProcess) {
    fprintf(stderr, "Unable to get handle.");
    getchar();
    return 1;
  }
  // allocate memory
  code_cave = VirtualAllocEx(hProcess, NULL, LOG_AND_JMP_BACK_PAYLOAD_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  if (!code_cave) {
    fprintf(stderr, "Unable to allocate memory.");
    getchar();
    return 1;
  }
  // get dump address
  dump_addr = GetProcAddress(GetModuleHandle("nexustk-logger.dll"), "dump");
  if (!dump_addr) {
    fprintf(stderr, "Unable to get address of dump function.");
    getchar();
    return 1;
  }
  jmp_to_log_payload = build_jmp_to_log_payload(calculate_relative_address(code_cave, PATCH_SRC));
  log_and_jmp_back_payload = build_log_and_jmp_back_payload(dump_addr, calculate_relative_address(PATCH_SRC + 0x05, code_cave + 0x1F));
  WriteProcessMemory(hProcess, (LPVOID)PATCH_SRC, jmp_to_log_payload, JMP_TO_LOG_PAYLOAD_SIZE, NULL);
  WriteProcessMemory(hProcess, code_cave, log_and_jmp_back_payload, LOG_AND_JMP_BACK_PAYLOAD_SIZE, NULL);
  printf("Payload written at %p\n\n", code_cave);
  return 0;
}

BOOL WINAPI DllMain(HINSTANCE hInstDll, DWORD dwReason, LPVOID lpvReserved) {
  switch (dwReason) {
    case DLL_PROCESS_ATTACH: {
      init_console();
      dump(0, NULL); /* make compiler not get rid of dead code */
      write_payload();
      break;
    }
  }
  return TRUE;
}
