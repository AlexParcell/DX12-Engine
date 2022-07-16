#pragma once

#include "stdafx.h"

class D3DHandler
{
public:
	D3DHandler();
	D3DHandler(const D3DHandler& other);
	~D3DHandler();

	bool Initialise(int screenHeight, int screenWidth, HWND hwnd, bool vsync, bool fullscreen);
	void Shutdown();
	bool Render();

private:
	bool InitialiseCommandQueue();
	bool InitialiseSwapChain(int screenHeight, int screenWidth, HWND hwnd, bool fullscreen, int numerator, int denominator);
	bool InitialiseDisplayAdapter(int screenHeight, int screenWidth, HWND hwnd, bool vsync, bool fullscreen, unsigned int& numerator, unsigned int& denominator);
	bool InitialiseRenderTargets();

private:
	bool m_vsyncEnabled;
	char m_videoCardDesc[128];
	unsigned int m_bufferIndex;
	unsigned long long m_fenceValue;
	int m_videoCardMemory;

	ID3D12Device* m_device;
	ID3D12CommandQueue* m_commandQueue;
	IDXGISwapChain3* m_swapChain;
	ID3D12DescriptorHeap* m_renderTargetViewHeap;
	ID3D12Resource* m_backBufferRenderTarget[2];
	ID3D12CommandAllocator* m_commandAllocator;
	ID3D12GraphicsCommandList* m_commandList;
	ID3D12PipelineState* m_pipelineState;
	ID3D12Fence* m_fence;

	HANDLE m_fenceEvent;
};

