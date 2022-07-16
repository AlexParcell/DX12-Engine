#include "Application.h"

#include "GraphicsHandler.h"
#include "InputHandler.h"

#include <stdexcept>

Application::Application()
	: m_inputHandler(nullptr),
	m_graphicsHandler(nullptr),
	m_exit(false)
{
}

Application::Application(const Application& other)
{
}

Application::~Application()
{
}

void Application::Initialize()
{
	int screenHeight = 0;
	int screenWidth = 0;

	InitialiseWindows(screenHeight, screenWidth);

	m_inputHandler = new InputHandler();
	if (!m_inputHandler)
		throw std::runtime_error("Failed to allocate for input handler");

	m_inputHandler->Initialise();

	m_graphicsHandler = new GraphicsHandler();
	if (!m_graphicsHandler)
		throw std::runtime_error("Failed to allocate for graphics handler");

	m_graphicsHandler->Initialise(screenHeight, screenWidth, m_hwnd);
}

void Application::Run()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	while (!m_exit)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
			m_exit = true;
		else
			Update();
	}
}

void Application::Shutdown()
{
	if (m_graphicsHandler)
	{
		m_graphicsHandler->Shutdown();
		delete m_graphicsHandler;
		m_graphicsHandler = nullptr;
	}

	if (m_inputHandler)
	{
		delete m_inputHandler;
		m_inputHandler = nullptr;
	}

	ShutdownWindows();
}

LRESULT Application::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
	case WM_KEYDOWN:
	{
		m_inputHandler->KeyDown((unsigned int)wparam);
		return 0;
	}

	case WM_KEYUP:
	{
		m_inputHandler->KeyUp((unsigned int)wparam);
		return 0;
	}
	}

	return DefWindowProc(hwnd, umsg, wparam, lparam);
}

void Application::Update()
{
	if (m_inputHandler->IsKeyDown(VK_ESCAPE))
		m_exit = true;

	m_graphicsHandler->Update();
}

void Application::InitialiseWindows(int& height, int& width)
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;

	// Set global instance
	g_app = this;

	// Get instance of this application and name it
	m_hInstance = GetModuleHandle(NULL);
	m_appName = L"Alex's Really Fucking Cool Engine For Cool People Who Fuck";

	// Setup windows class with default settings
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_appName;
	wc.cbSize = sizeof(WNDCLASSEX);

	// Register our window
	RegisterClassEx(&wc);

	// Determine resolution of client desktop screen
	height = GetSystemMetrics(SM_CYSCREEN);
	width = GetSystemMetrics(SM_CXSCREEN);

	if (FULL_SCREEN)
	{
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsHeight = (unsigned long)height;
		dmScreenSettings.dmPelsWidth = (unsigned long)width;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		// Set the position of the window to the top left corner.
		posX = posY = 0;
	}
	else
	{
		// If windowed then set it to 800x600 resolution.
		width = 800;
		height = 600;

		// Place the window in the middle of the screen.
		posX = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
	}

	// Create the window with the screen settings and get the handle to it.
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_appName, m_appName,
		WS_OVERLAPPEDWINDOW,
		posX, posY, width, height, NULL, NULL, m_hInstance, NULL);

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	// Hide the mouse cursor.
	ShowCursor(false);
}

void Application::ShutdownWindows()
{
	// Show the mouse cursor.
	ShowCursor(true);

	// Fix the display settings if leaving full screen mode.
	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// Remove the window.
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	// Remove the application instance.
	UnregisterClass(m_appName, m_hInstance);
	m_hInstance = NULL;

	// Release the pointer to this class.
	g_app = nullptr;
}

LRESULT WndProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
		// Check if the window is being destroyed.
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}

	// Check if the window is being closed.
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
	}

	// All other messages pass to the message handler in the system class.
	default:
	{
		return g_app->MessageHandler(hwnd, umsg, wparam, lparam);
	}
	}
}
