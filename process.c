#include <psapi.h>
#include <string.h>

int cmp_process_name(DWORD processID, const char *targetName) {
  HANDLE hProcess;
  HMODULE hMod;
  DWORD cbNeeded;
  TCHAR szProcessName[MAX_PATH];
  hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
  EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded);
  GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName));
  CloseHandle(hProcess);
  return strcmp(szProcessName, targetName);
}

DWORD find_process_id(const char *name) {
  DWORD aProcesses[1024];
  DWORD cbNeeded;
  DWORD cProcesses;
  unsigned int i;
  EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded);
  cProcesses = cbNeeded / sizeof(DWORD);
  for (i = 0; i < cProcesses; i++) {
    if (!aProcesses[i]) {
      continue;
    }
    if (cmp_process_name(aProcesses[i], name) == 0) {
      return aProcesses[i];
    }
  }
  return 0;
}

HANDLE get_handle() {
  DWORD processID;
  HANDLE hProcess;
  processID = find_process_id("NexusTK.exe");
  if (!processID) {
    fprintf(stderr, "Unable to find process.");
    return NULL;
  }
  hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_QUERY_INFORMATION | PROCESS_VM_WRITE | PROCESS_CREATE_THREAD, FALSE, processID);
  if (!hProcess) {
    fprintf(stderr, "Unable to open process.");
    return NULL;
  }
  return hProcess;
}
