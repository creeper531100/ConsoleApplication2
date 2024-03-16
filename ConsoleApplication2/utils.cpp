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

void type_string(const std::string& str) {
    INPUT ip;
    ZeroMemory(&ip, sizeof(INPUT));

    ip.type = INPUT_KEYBOARD;
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;

    for (const char& c : str) {
        ip.ki.wVk = 0;
        ip.ki.wScan = c;
        ip.ki.dwFlags = KEYEVENTF_UNICODE; // Use Unicode
        SendInput(1, &ip, sizeof(INPUT));

        ip.ki.dwFlags |= KEYEVENTF_KEYUP; // Key release
        SendInput(1, &ip, sizeof(INPUT));
    }
}


void click(int x, int y) {
    INPUT ip;
    ZeroMemory(&ip, sizeof(INPUT));

    ip.type = INPUT_MOUSE;
    ip.mi.dx = x * (65536 / GetSystemMetrics(SM_CXSCREEN));//x being coord in pixels
    ip.mi.dy = y * (65536 / GetSystemMetrics(SM_CYSCREEN));//y being coord in pixels

    ip.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &ip, sizeof(INPUT));

    ip.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &ip, sizeof(INPUT));
}
