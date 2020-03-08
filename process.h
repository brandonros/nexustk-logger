#ifndef PROCESS_H
#define PROCESS_H
#define PATCH_SRC 0x004C0BA0
#define PATCH_DEST 0x004C0BA5
int cmp_process_name(DWORD processID, const char *targetName);
DWORD find_process_id(const char *name);
HANDLE get_handle();
#endif
