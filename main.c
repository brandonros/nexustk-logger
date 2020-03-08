#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <string.h>
#include "process.h"

DWORD inject_dll(HANDLE hProcess, LPVOID param) {
  LPVOID libAddr;
  HANDLE remoteThread;
  libAddr = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
  if (!libAddr) {
    fprintf(stderr, "Unable to get library address.");
    return 0;
  }
  remoteThread = (LPVOID)CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)libAddr, param, 0, NULL);
  if (!remoteThread) {
    fprintf(stderr, "Unable to create remote thread.");
    return 0;
  }
  return 1;
}

int main(int argc, char *argv[]) {
  DWORD processID;
  HANDLE hProcess;
  LPVOID mem;
  const char path[] = "nexustk-logger.dll";
  processID = find_process_id("NexusTK.exe");
  if (!processID) {
    fprintf(stderr, "Unable to find process.");
    return 1;
  }
  hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_QUERY_INFORMATION | PROCESS_VM_WRITE | PROCESS_CREATE_THREAD, FALSE, processID);
  if (!hProcess) {
    fprintf(stderr, "Unable to open process.");
    return 1;
  }
  mem = VirtualAllocEx(hProcess, NULL, strlen(path), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  if (!mem) {
    fprintf(stderr, "Unable to allocate memory.");
    return 1;
  }
  WriteProcessMemory(hProcess, mem, path, strlen(path), NULL);
  inject_dll(hProcess, mem);
  CloseHandle(hProcess);
  return 0;
}
