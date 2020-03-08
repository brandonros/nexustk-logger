#ifndef PROCESS_H
#define PROCESS_H

#define PATCH_SRC 0x005BEF90  // 2020/03/08, client EXE version 7.50

int cmp_process_name(DWORD, const char*);
DWORD find_process_id(const char*);
HANDLE get_handle();

#endif
