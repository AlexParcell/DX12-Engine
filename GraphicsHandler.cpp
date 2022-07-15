#include "GraphicsHandler.h"

#include "D3DHandler.h"

GraphicsHandler::GraphicsHandler()
{
	m_d3d = nullptr;
}

GraphicsHandler::GraphicsHandler(const GraphicsHandler& other)
{
}

GraphicsHandler::~GraphicsHandler()
{
}

bool GraphicsHandler::Initialise(int height, int width, HWND hwnd)
{
	m_d3d = new D3DHandler();
	if (!m_d3d)
		return false;

	bool success = m_d3d->Initialise(height, width, hwnd, VSYNC_ENABLED, FULL_SCREEN);
	if (!success)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}

	return true;
}

void GraphicsHandler::Shutdown()
{
	if (m_d3d)
	{
		m_d3d->Shutdown();
		delete m_d3d;
		m_d3d = nullptr;
	}
}

bool GraphicsHandler::Update()
{
	if (!Render()) return false;

	return true;
}

bool GraphicsHandler::Render()
{
	if (!m_d3d->Render()) return false;

	return true;
}
