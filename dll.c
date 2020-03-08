#include <windows.h>
#include <stdio.h>
#include "process.h"

void init_console() {
  AllocConsole();
  freopen("CONOUT$", "w", stdout);
  printf("DLL loaded!\n");
}

__declspec(dllexport) void dump(int size, const unsigned char *buf) {
  int i;
  for (i = 0; i < size; ++i) {
    printf("%02x ", buf[i]);
  }
  printf("\n\n");
}

DWORD write_payload() {
  HANDLE hProcess;
  LPVOID mem;
  unsigned char asm_payload[] = {
    0x60, /* pusha */
    0x9C, /* pushf */
    0xFF, 0x74, 0x24, 0x28, /* push DWORD PTR [esp+0x28] */
    0xFF, 0x74, 0x24, 0x30, /* push DWORD PTR [esp+0x30] */
    0xB8, 0x00, 0x00, 0x00, 0x00, /* mov eax, 0x000000; address gets replaced dynamically later */
    0xFF, 0xD0, /* call eax */
    0x83, 0xC4, 0x08, /* add esp, 0x08 */
    0x9D, /* popf */
    0x61, /* popd */
    0x81, 0xEC, 0x1C, 0x01, 0x00, 0x00, /* sub esp, 0x11c */
    0xE9, 0x00, 0x00, 0x00, 0x00 /* jmp 0x00000000; relative address gets replaced dynamically later */
  };
  unsigned char jmp_payload[] = {
    0xE9, 0x00, 0x00, 0x00, 0x00, /* jmp 0x00000000; relative address gets replaced dynamically later */
    0x90 /* nop */
  };
  unsigned int src = PATCH_SRC;
  unsigned int dest = PATCH_DEST;
  LPVOID dump_addr = GetProcAddress(GetModuleHandle("nexustk.dll"), "dump");
  unsigned int diff;
  hProcess = get_handle();
  if (!hProcess) {
    return 1;
  }
  mem = VirtualAllocEx(hProcess, NULL, sizeof(asm_payload), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  if (!mem) {
    fprintf(stderr, "Unable to allocate memory.");
    return 1;
  }
  diff = (unsigned int)mem - src - 5;
  memcpy(jmp_payload + 1, &diff, 4);
  WriteProcessMemory(hProcess, (LPVOID)src, jmp_payload, sizeof(jmp_payload), NULL);
  memcpy(asm_payload + 11, &dump_addr, 4);
  diff = dest - ((unsigned int)mem + 0x1C) - 5;
  memcpy(asm_payload + 29, &diff, 4);
  WriteProcessMemory(hProcess, mem, asm_payload, sizeof(asm_payload), NULL);
  printf("Payload written at %p\n\n", mem);
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