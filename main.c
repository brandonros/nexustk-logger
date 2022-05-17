#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <string.h>
#include "process.h"

int main(int argc, char *argv[]) {
  DWORD processID;
  HANDLE hProcess;
  LPVOID libAddr;
  HANDLE remoteThread;
  LPVOID mem;
  const char path[] = "C:\\Windows\\System32\\nexustk-logger.dll";
  // find process ID
  processID = find_process_id("NexusTK.exe");
  if (!processID) {
    fprintf(stderr, "Unable to find process.\n");
    getchar();
    return 1;
  }
  printf("Found NexusTK.exe process ID.\n");
  // open process
  hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_QUERY_INFORMATION | PROCESS_VM_WRITE | PROCESS_CREATE_THREAD, FALSE, processID);
  if (!hProcess) {
    fprintf(stderr, "Unable to open process.\n");
    getchar();
    return 1;
  }
  printf("Got NexusTK.exe process handle.\n");
  // allocate memory
  mem = VirtualAllocEx(hProcess, NULL, strlen(path) + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  if (!mem) {
    fprintf(stderr, "Unable to allocate memory.\n");
    getchar();
    return 1;
  }
  printf("Allocated memory.\n");
  // write DLL path
  WriteProcessMemory(hProcess, mem, path, strlen(path) + 1, NULL);
  printf("Wrote DLL path to memory.\n");
  // find LoadLibraryA address
  libAddr = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
  if (!libAddr) {
    fprintf(stderr, "Unable to get library address.\n");
    getchar();
    return 1;
  }
  printf("Found LoadLibraryA address.\n");
  // create remote thread
  remoteThread = (LPVOID)CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)libAddr, mem, 0, NULL);
  if (!remoteThread) {
    fprintf(stderr, "Unable to create remote thread.\n");
    getchar();
    return 1;
  }
  printf("Spawned remote thread.\n");
  // cleanup
  CloseHandle(hProcess);
  printf("Closed handle.\n");
  return 0;
}
