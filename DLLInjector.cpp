#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <stdio.h>

DWORD GetProcessIdByName(const TCHAR* processName) {
    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (Process32First(snapshot, &processEntry)) {
        do {
            if (_tcsicmp(processEntry.szExeFile, processName) == 0) {
                CloseHandle(snapshot);
                return processEntry.th32ProcessID;
            }
        } while (Process32Next(snapshot, &processEntry));
    }

    CloseHandle(snapshot);
    return 0;
}

HANDLE OpenTargetProcess(const TCHAR* processName) {
    DWORD processId = GetProcessIdByName(processName);
    if (processId == 0) {
        printf("Target process not found.\n");
        return NULL;
    }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (hProcess == NULL) {
        printf("Failed to open target process.\n");
    }
    return hProcess;
}

LPVOID AllocateMemoryInTargetProcess(HANDLE hProcess, const char* dllPath) {
    LPVOID pRemoteDllPath = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
    if (pRemoteDllPath == NULL) {
        printf("Failed to allocate memory in target process.\n");
    }
    return pRemoteDllPath;
}

BOOL WriteDllPathToTargetProcess(HANDLE hProcess, LPVOID pRemoteDllPath, const char* dllPath) {
    if (!WriteProcessMemory(hProcess, pRemoteDllPath, dllPath, strlen(dllPath) + 1, NULL)) {
        printf("Failed to write DLL path to target process.\n");
        return FALSE;
    }
    return TRUE;
}

BOOL CreateRemoteThreadInTargetProcess(HANDLE hProcess, LPVOID pRemoteDllPath) {
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, pRemoteDllPath, 0, NULL);
    if (hThread == NULL) {
        printf("Failed to create remote thread in target process.\n");
        return FALSE;
    }

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    return TRUE;
}

int main() {
    const TCHAR* targetProcessName = _T("notepad.exe");
    const char* dllPath = "C:\\Users\\minhvt13\\source\\repos\\DLL\\x64\\Debug\\DLL.dll";

    HANDLE hProcess = OpenTargetProcess(targetProcessName);
    if (hProcess == NULL) {
        return 1;
    }

    LPVOID pRemoteDllPath = AllocateMemoryInTargetProcess(hProcess, dllPath);
    if (pRemoteDllPath == NULL) {
        CloseHandle(hProcess);
        return 1;
    }

    if (!WriteDllPathToTargetProcess(hProcess, pRemoteDllPath, dllPath)) {
        VirtualFreeEx(hProcess, pRemoteDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    if (!CreateRemoteThreadInTargetProcess(hProcess, pRemoteDllPath)) {
        VirtualFreeEx(hProcess, pRemoteDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    VirtualFreeEx(hProcess, pRemoteDllPath, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    printf("DLL injected successfully.\n");
    return 0;
}