#include "stdafx.h"
#include "Application.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	Application* app = new Application();

	app->Initialize();
	app->Run();
	app->Shutdown();

	delete app;
	app = nullptr;
	
	return 0;
}