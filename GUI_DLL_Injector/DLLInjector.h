#pragma once

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>

struct Process
{
	DWORD id;
	std::string name;
};

class DLLInjector
{
private:
	static DWORD GetProcID(const char* procName);

public:
	static std::vector<Process> getProcList();
	static bool Inject(const char* procName, const char* dllPath);
	static bool Inject(DWORD procID, const char* dllPath);
};