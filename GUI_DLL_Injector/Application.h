#pragma once

#include "imgui.h"
#include "DLLInjector.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <algorithm>
#include <exception>

namespace Global
{
	inline std::vector<Process> procList;
	inline bool loadList = false;

	inline char dllPath[MAX_PATH] = "";
	inline std::string fixedDllPath = "";
}

class App
{
private:
	static std::string replace_all(std::string& str, const std::string& find, const std::string& replace);

public:
	static std::string selectFile();
	static void renderUI();
};

