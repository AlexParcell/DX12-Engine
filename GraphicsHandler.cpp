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

void GraphicsHandler::Initialise(int height, int width, HWND hwnd)
{
	m_d3d = new D3DHandler();

	m_d3d->Initialise(height, width, hwnd, VSYNC_ENABLED, FULL_SCREEN);
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

void GraphicsHandler::Update()
{
	Render();
}

void GraphicsHandler::Render()
{
	m_d3d->Render();
}
