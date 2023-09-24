#pragma once

#include <Windows.h>
#include <ShellAPI.h>
#include "imgui.h"

#include <glad/glad.h>
#include <gl/glcorearb.h>

#include "DLLInjector.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <algorithm>
#include <exception>
#include <cctype>
#include <Psapi.h>
#include <tchar.h>
#include <codecvt>

namespace Global
{
	inline const int GL_WIN_WIDTH = 800;
	inline const int GL_WIN_HEIGHT = 600;

	inline std::vector<Process> procList;
	inline Process selectedProcess;
	inline bool loadList = false;

	inline char dllPath[MAX_PATH] = "";
	inline std::string fixedDllPath = "";

	inline bool procWatching = false;
	inline char procWatched[MAX_PATH] = "";
	inline char procFilter[MAX_PATH] = "";
	inline bool procSorted = false;

	inline bool closeOnInject = false;
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
	static GLuint CreateTextureFromHICON(HICON& hIcon);

	static bool getProcessIcon(DWORD processId, HICON& phiconLarge, HICON& phiconSmall);
	static void renderImageWithImGui(const std::vector<unsigned char>& imageData, int width, int height);
	static void loadProcList();
	static void sortProcList();
	static void loadProcTable();
	static void renderProcWnd(const OpenGL& window);

	static void watchProcess();
	static void renderInjectionWnd(const OpenGL& window);

public:
	static std::string selectFile();
	static void renderUI(const OpenGL& window);
};