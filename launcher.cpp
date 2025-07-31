#include <windows.h>
#include <detours/detours.h>

#define BUFSIZE 4096

int main()
{
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    DWORD dwFlags = CREATE_SUSPENDED;
    TCHAR dll[BUFSIZE]=TEXT("");
    GetFullPathName(TEXT("hook32.dll"), BUFSIZE, dll, NULL);
    LPCSTR rpszDllsOut[1] = {dll};
    DetourCreateProcessWithDlls("C:/megamari/megamari_e.exe", NULL,
                                NULL, NULL, TRUE, dwFlags, NULL, "C:/megamari",
                                &si, &pi, 1, rpszDllsOut, NULL);

    ResumeThread(pi.hThread);
    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}