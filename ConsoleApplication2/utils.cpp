#include "utils.h"
using namespace std;

HANDLE GetProcessByName(std::wstring name, ProcessWindowInfo* info) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 process;
    ZeroMemory(&process, sizeof(process));
    process.dwSize = sizeof(process);
    if (Process32First(snapshot, &process)) {
        do {
            if (wstring(process.szExeFile) == name) {
                info->pid = process.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &process));
    }
    CloseHandle(snapshot);

    if (info->pid != 0) {
        return OpenProcess(PROCESS_ALL_ACCESS, FALSE, info->pid);
    }
    return NULL;
}

BOOL enumWindowCallback(HWND hWnd, LPARAM lparam) {
    DWORD dwProcessId = 0;
    GetWindowThreadProcessId(hWnd, &dwProcessId);

    ProcessWindowInfo* s = (ProcessWindowInfo*)lparam;
    if (s->pid == dwProcessId && IsWindowVisible(hWnd)) {
        s->hwnd = hWnd;
        return FALSE;
    }
    return TRUE;
}
