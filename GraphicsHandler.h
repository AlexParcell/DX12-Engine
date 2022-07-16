#pragma once

#include "stdafx.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

class D3DHandler;

class GraphicsHandler
{
	D3DHandler* m_d3d;

public:
	GraphicsHandler();
	GraphicsHandler(const GraphicsHandler& other);
	~GraphicsHandler();

	void Initialise(int, int, HWND);
	void Shutdown();
	void Update();

private:
	void Render();
};

