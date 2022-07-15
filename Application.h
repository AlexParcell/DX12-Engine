#pragma once

#include "stdafx.h"

class GraphicsHandler;
class InputHandler;

class Application
{
public:
	Application();
	Application(const Application& other);
	~Application();

	bool Initialize();
	void Run();
	void Shutdown();

	LRESULT CALLBACK MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

private:
	bool Update();
	void InitialiseWindows(int& height, int& width);
	void ShutdownWindows();

private:
	LPCWSTR m_appName;
	HINSTANCE m_hInstance;
	HWND m_hwnd;

	GraphicsHandler* m_graphicsHandler;
	InputHandler* m_inputHandler;
};

static LRESULT CALLBACK WndProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

static Application* g_app = nullptr;
