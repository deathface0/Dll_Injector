#pragma once

#include "imgui.h"
#include "DLLInjector.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <algorithm>
#include <exception>
#include <cctype>

namespace Global
{
	inline const int GL_WIN_WIDTH = 800;
	inline const int GL_WIN_HEIGHT = 600;

	inline std::vector<Process> procList;
	inline bool loadList = false;

	inline char dllPath[MAX_PATH] = "";
	inline std::string fixedDllPath = "";

	inline bool procWatching = false;
	inline char procWatched[MAX_PATH] = "";
	inline char procFilter[MAX_PATH] = "";
}

struct OpenGL
{
	int windowX;
	int windowY;
};

class App
{
private:
	static std::string replace_all(std::string& str, const std::string& find, const std::string& replace);

	static void loadProcList();
	static void loadProcTable();
	static void renderProcWnd(const OpenGL& window);

	static void watchProcess();
	static void renderInjectionWnd(const OpenGL& window);

public:
	static std::string selectFile();
	static void renderUI(const OpenGL& window);
};

