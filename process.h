#ifndef PROCESS_H
#define PROCESS_H
#include <stdint.h>

// 2020/03/08, client EXE version 7.50
static uint8_t original_instructions[] = {
  // push ebp
  0x55,
  // mov ebp, esp
  0x8B, 0xEC,
  // sub esp, 0x164
  0x81, 0xEC, 0x64, 0x01, 0x00, 0x00
};
#define ORIGINAL_INSTRUCTIONS_SIZE sizeof(original_instructions)
#define PATCH_SRC 0x005BEF90
#define ARG_1_STACK_OFFSET 0x08
#define ARG_2_STACK_OFFSET 0x0C
#define NUM_ARGS 2

int cmp_process_name(DWORD, const char*);
DWORD find_process_id(const char*);
HANDLE get_handle();

#endif
