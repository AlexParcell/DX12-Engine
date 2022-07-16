#pragma once

#include "stdafx.h"

#include <stdexcept>

inline void ThrowIfFailed(HRESULT hr) { if (FAILED(hr)) throw std::runtime_error("Encountered runtime failure."); }

class D3DHandler
{
public:
	D3DHandler();
	D3DHandler(const D3DHandler& other);
	~D3DHandler();

	void Initialise(int screenHeight, int screenWidth, HWND hwnd, bool vsync, bool fullscreen);
	void Shutdown();

	void Render();

private:
	void InitialiseCommandQueue();
	void InitialiseSwapChain(int screenHeight, int screenWidth, HWND hwnd, bool fullscreen, int numerator, int denominator);
	void InitialiseDisplayAdapter(int screenHeight, int screenWidth, HWND hwnd, bool vsync, bool fullscreen, unsigned int& numerator, unsigned int& denominator);
	void InitialiseRenderTargets();

private:
	bool m_vsyncEnabled = true;
	char m_videoCardDesc[128];
	unsigned int m_bufferIndex = 0;
	unsigned long long m_fenceValue = 0;
	int m_videoCardMemory = 0;

	ID3D12Device* m_device = nullptr;
	ID3D12CommandQueue* m_commandQueue = nullptr;
	IDXGISwapChain3* m_swapChain = nullptr;
	ID3D12DescriptorHeap* m_renderTargetViewHeap = nullptr;
	ID3D12Resource* m_backBufferRenderTarget[2] = {};
	ID3D12CommandAllocator* m_commandAllocator = nullptr;
	ID3D12GraphicsCommandList* m_commandList = nullptr;
	ID3D12PipelineState* m_pipelineState = nullptr;
	ID3D12Fence* m_fence = nullptr;

	HANDLE m_fenceEvent;
};

