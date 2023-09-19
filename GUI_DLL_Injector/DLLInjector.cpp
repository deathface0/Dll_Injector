#include "DLLInjector.h"
#include <utility>

std::vector<Process> DLLInjector::getProcList()
{
    std::vector<Process> procList;

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnap != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 pEntry;
        pEntry.dwSize = sizeof(pEntry);

        if (Process32First(hSnap, &pEntry))
        {
            do {
                procList.push_back({ pEntry.th32ProcessID, pEntry.szExeFile });
            } while (Process32Next(hSnap, &pEntry));
        }
    }
    CloseHandle(hSnap);

    return procList;
}

DWORD DLLInjector::GetProcID(const char* procName)
{
    DWORD procID = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnap != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 pEntry;
        pEntry.dwSize = sizeof(pEntry);

        if (Process32First(hSnap, &pEntry))
        {
            do {
                if (!_stricmp(pEntry.szExeFile, procName))
                {
                    procID = pEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnap, &pEntry));
        }

    }
    CloseHandle(hSnap);
    return procID;
}


bool DLLInjector::Inject(const char* procName, const char* dllPath)
{
    DWORD procID = GetProcID(procName);

    if (!procID) 
    { 
        std::cout << "Process not found!" << std::endl; 
        return 0;
    }
    
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, procID);

    if (!hProc || hProc == INVALID_HANDLE_VALUE) { return 0; }
    
    void* loc = VirtualAllocEx(hProc, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (loc) { WriteProcessMemory(hProc, loc, dllPath, strlen(dllPath) + 1, 0); }
    else { std::cout << "Cannot write mem" << std::endl; return 0; }
    
    HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, loc, 0, 0);
    if (hThread) { CloseHandle(hThread); }
    else { std::cout << "Cannot create remote thread" << std::endl; return 0; }

    if (hProc) { CloseHandle(hProc); }
    else { std::cout << "Process handle not found" << std::endl; return 0; }

    return 1;
}

bool DLLInjector::Inject(DWORD procID, const char* dllPath)
{
    if (!procID)
    {
        std::cout << "Process not found!" << std::endl;
        return 0;
    }

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, procID);

    if (!hProc || hProc == INVALID_HANDLE_VALUE) { return 0; }

    void* loc = VirtualAllocEx(hProc, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (loc) { WriteProcessMemory(hProc, loc, dllPath, strlen(dllPath) + 1, 0); }
    else { std::cout << "Cannot write mem" << std::endl; return 0; }

    HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, loc, 0, 0);
    if (hThread) { CloseHandle(hThread); }
    else { std::cout << "Cannot create remote thread" << std::endl; return 0; }

    if (hProc) { CloseHandle(hProc); }
    else { std::cout << "Process handle not found" << std::endl; return 0; }

    return 1;
}
