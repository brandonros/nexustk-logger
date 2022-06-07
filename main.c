#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <string.h>
#include "process.h"

int main(int argc, char *argv[]) {
  DWORD processId;
  HANDLE hProcess;
  LPVOID loadLibraryAddr;
  HANDLE remoteThread;
  LPVOID mem;
  const char loggerDllPath[] = "C:\\Program Files (x86)\\KRU\\NexusTK\\nexustk-logger.dll";
  // find process ID
  processId = find_process_id("NexusTK.exe");
  if (!processId) {
    fprintf(stderr, "Unable to find process.\n");
    getchar();
    return 1;
  }
  printf("Found NexusTK.exe process ID.\n");
  // open process
  hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_QUERY_INFORMATION | PROCESS_VM_WRITE | PROCESS_CREATE_THREAD, FALSE, processId);
  if (!hProcess) {
    fprintf(stderr, "Unable to open process.\n");
    getchar();
    return 1;
  }
  printf("Got NexusTK.exe process handle.\n");
  // allocate memory
  mem = VirtualAllocEx(hProcess, NULL, strlen(loggerDllPath) + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  if (!mem) {
    fprintf(stderr, "Unable to allocate memory.\n");
    getchar();
    return 1;
  }
  printf("Allocated memory.\n");
  // write DLL path
  WriteProcessMemory(hProcess, mem, loggerDllPath, strlen(loggerDllPath) + 1, NULL);
  printf("Wrote DLL path to memory.\n");
  // find LoadLibraryA address
  loadLibraryAddr = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
  if (!loadLibraryAddr) {
    fprintf(stderr, "Unable to get library address.\n");
    getchar();
    return 1;
  }
  printf("Found LoadLibraryA address.\n");
  // create remote thread
  remoteThread = (LPVOID)CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddr, mem, 0, NULL);
  if (!remoteThread) {
    fprintf(stderr, "Unable to create remote thread.\n");
    getchar();
    return 1;
  }
  printf("Spawned remote thread.\n");
  // wait
  WaitForSingleObject(remoteThread, INFINITE);
  printf("Remote thread ended.\n");
  // get exit code
  DWORD exitCode = 0;
  if (GetExitCodeThread(remoteThread, &exitCode) == 0) {
    fprintf(stderr, "Unable to get exit code thread.\n");
    getchar();
    return 1;
  }
  printf("exitCode = %d\n", exitCode);
  // cleanup
  VirtualFreeEx(hProcess, mem, 0, MEM_RELEASE);
  CloseHandle(hProcess);
  printf("Closed handle.\n");
  return 0;
}
