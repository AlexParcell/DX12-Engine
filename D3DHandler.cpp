#include "D3DHandler.h"

D3DHandler::D3DHandler()
{
}

D3DHandler::D3DHandler(const D3DHandler& other)
{
}

D3DHandler::~D3DHandler()
{
}

void D3DHandler::InitialiseCommandQueue()
{
	// Initialize the description of the command queue.
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
	ZeroMemory(&commandQueueDesc, sizeof(commandQueueDesc));

	// Set up the description of the command queue.
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.NodeMask = 0;

	// Create the command queue.
	ThrowIfFailed(m_device->CreateCommandQueue(&commandQueueDesc, __uuidof(ID3D12CommandQueue), (void**)&m_commandQueue));
}

void D3DHandler::InitialiseSwapChain(int screenHeight, int screenWidth, HWND hwnd, bool fullscreen, int numerator, int denominator)
{
	// Create a DirectX graphics interface factory.
	IDXGIFactory4* factory;
	ThrowIfFailed(CreateDXGIFactory1(__uuidof(IDXGIFactory4), (void**)&factory));

	// Initialize the swap chain description.
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// Set the swap chain to use double buffering.
	swapChainDesc.BufferCount = 2;

	// Set the height and width of the back buffers in the swap chain.
	swapChainDesc.BufferDesc.Height = screenHeight;
	swapChainDesc.BufferDesc.Width = screenWidth;

	// Set a regular 32-bit surface for the back buffers.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Set the usage of the back buffers to be render target outputs.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// Set the swap effect to discard the previous buffer contents after swapping.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	// Set the handle for the window to render to.
	swapChainDesc.OutputWindow = hwnd;

	// Set to full screen or windowed mode.
	swapChainDesc.Windowed = !fullscreen;

	// Set the refresh rate of the back buffer
	swapChainDesc.BufferDesc.RefreshRate.Numerator = m_vsyncEnabled ? numerator : 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = m_vsyncEnabled ? denominator : 1;

	// Turn multisampling off.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// Set the scan line ordering and scaling to unspecified.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Don't set the advanced flags.
	swapChainDesc.Flags = 0;

	// Finally create the swap chain using the swap chain description.
	IDXGISwapChain* swapChain = nullptr;
	ThrowIfFailed(factory->CreateSwapChain(m_commandQueue, &swapChainDesc, &swapChain));

	// Next upgrade the IDXGISwapChain to a IDXGISwapChain3 interface and store it in a private member variable named m_swapChain.
	// This will allow us to use the newer functionality such as getting the current back buffer index.
	ThrowIfFailed(swapChain->QueryInterface(__uuidof(IDXGISwapChain3), (void**)&m_swapChain));

	// Clear pointer to original swap chain interface since we are using version 3 instead (m_swapChain).
	swapChain = nullptr;

	// Release the factory now that the swap chain has been created.
	factory->Release();
	factory = nullptr;
}

void D3DHandler::InitialiseDisplayAdapter(int screenHeight, int screenWidth, HWND hwnd, bool vsync, bool fullscreen, unsigned int& numerator, unsigned int& denominator)
{
	// Create a DirectX graphics interface factory.
	IDXGIFactory4* factory;
	ThrowIfFailed(CreateDXGIFactory1(__uuidof(IDXGIFactory4), (void**)&factory));

	// Use the factory to create an adapter for the primary graphics interface (video card).
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput;

	ThrowIfFailed(factory->EnumAdapters(0, &adapter));

	// Enumerate the primary adapter output (monitor).
	ThrowIfFailed(adapter->EnumOutputs(0, &adapterOutput));

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	unsigned int numModes;
	ThrowIfFailed(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL));

	// Create a list to hold all the possible display modes for this monitor/video card combination.
	DXGI_MODE_DESC* displayModeList = new DXGI_MODE_DESC[numModes];

	// Now fill the display mode list structures.
	ThrowIfFailed(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList));

	// Now go through all the display modes and find the one that matches the screen height and width.
	// When a match is found store the numerator and denominator of the refresh rate for that monitor.
	for (unsigned int i = 0; i < numModes; i++)
	{
		DXGI_MODE_DESC* currentMode = displayModeList + i;
		if (currentMode->Height == (unsigned int)screenHeight)
		{
			if (currentMode->Width == (unsigned int)screenWidth)
			{
				numerator = currentMode->RefreshRate.Numerator;
				denominator = currentMode->RefreshRate.Denominator;
			}
		}
	}

	// Get the adapter (video card) description.
	DXGI_ADAPTER_DESC adapterDesc;
	ThrowIfFailed(adapter->GetDesc(&adapterDesc));

	// Store the dedicated video card memory in megabytes.
	m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// Convert the name of the video card to a character array and store it.
	unsigned long long stringLength;
	wcstombs_s(&stringLength, m_videoCardDesc, 128, adapterDesc.Description, 128);

	// Release the display mode list.
	delete[] displayModeList;
	displayModeList = nullptr;

	// Release the adapter output.
	adapterOutput->Release();
	adapterOutput = nullptr;

	// Release the adapter.
	adapter->Release();
	adapter = nullptr;

	// Release the factory now that the swap chain has been created.
	factory->Release();
	factory = nullptr;
}

void D3DHandler::InitialiseRenderTargets()
{
	// Initialize the render target view heap description for the two back buffers.
	D3D12_DESCRIPTOR_HEAP_DESC renderTargetViewHeapDesc;
	ZeroMemory(&renderTargetViewHeapDesc, sizeof(renderTargetViewHeapDesc));

	// Set the number of descriptors to two for our two back buffers.  Also set the heap tyupe to render target views.
	renderTargetViewHeapDesc.NumDescriptors = 2;
	renderTargetViewHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	renderTargetViewHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	// Create the render target view heap for the back buffers.
	ThrowIfFailed(m_device->CreateDescriptorHeap(&renderTargetViewHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_renderTargetViewHeap));

	// Get a handle to the starting memory location in the render target view heap to identify where the render target views will be located for the two back buffers.
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle = m_renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();

	// Get the size of the memory location for the render target view descriptors.
	unsigned int renderTargetViewDescriptorSize = 0;
	renderTargetViewDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// Get a pointer to the first back buffer from the swap chain.
	ThrowIfFailed(m_swapChain->GetBuffer(0, __uuidof(ID3D12Resource), (void**)&m_backBufferRenderTarget[0]));

	// Create a render target view for the first back buffer.
	m_device->CreateRenderTargetView(m_backBufferRenderTarget[0], NULL, renderTargetViewHandle);

	// Increment the view handle to the next descriptor location in the render target view heap.
	renderTargetViewHandle.ptr += renderTargetViewDescriptorSize;

	// Get a pointer to the second back buffer from the swap chain.
	ThrowIfFailed(m_swapChain->GetBuffer(1, __uuidof(ID3D12Resource), (void**)&m_backBufferRenderTarget[1]));

	// Create a render target view for the second back buffer.
	m_device->CreateRenderTargetView(m_backBufferRenderTarget[1], NULL, renderTargetViewHandle);

	// Finally get the initial index to which buffer is the current back buffer.
	m_bufferIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void D3DHandler::Initialise(int screenHeight, int screenWidth, HWND hwnd, bool vsync, bool fullscreen)
{
	m_vsyncEnabled = vsync;

	// Some cards may not support this and need to be reduced to 12.0, this is 12.1
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_12_1;

	ThrowIfFailed(D3D12CreateDevice(NULL, featureLevel, __uuidof(ID3D12Device), (void**)&m_device));

	InitialiseCommandQueue();
	unsigned int numerator, denominator;
	InitialiseDisplayAdapter(screenHeight, screenWidth, hwnd, vsync, fullscreen, numerator, denominator);
	InitialiseSwapChain(screenHeight, screenWidth, hwnd, fullscreen, numerator, denominator);
	InitialiseRenderTargets();

	// Create a command allocator.
	ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&m_commandAllocator));

	// Create a basic command list.
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&m_commandList));

	// Initially we need to close the command list during initialization as it is created in a recording state.
	ThrowIfFailed(m_commandList->Close());

	// Create a fence for GPU synchronization.
	ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&m_fence));

	// Create an event object for the fence.
	m_fenceEvent = CreateEventEx(NULL, FALSE, FALSE, EVENT_ALL_ACCESS);

	// Initialize the starting fence value. 
	m_fenceValue = 1;
}

void D3DHandler::Shutdown()
{
	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	if (m_swapChain) m_swapChain->SetFullscreenState(false, NULL);

	// Close the object handle to the fence event.
	CloseHandle(m_fenceEvent);

	// Release the fence.
	if (m_fence) m_fence->Release();

	// Release the empty pipe line state.
	if (m_pipelineState) m_pipelineState->Release();

	// Release the command list.
	if (m_commandList) m_commandList->Release();

	// Release the command allocator.
	if (m_commandAllocator) m_commandAllocator->Release();

	// Release the back buffer render target views.
	if (m_backBufferRenderTarget[0]) m_backBufferRenderTarget[0]->Release();
	if (m_backBufferRenderTarget[1]) m_backBufferRenderTarget[1]->Release();

	// Release the render target view heap.
	if (m_renderTargetViewHeap) m_renderTargetViewHeap->Release();

	// Release the swap chain.
	if (m_swapChain) m_swapChain->Release();

	// Release the command queue.
	if (m_commandQueue) m_commandQueue->Release();

	// Release the device.
	if (m_device) m_device->Release();
}

void D3DHandler::Render()
{
	// Reset (re-use) the memory associated command allocator.
	ThrowIfFailed(m_commandAllocator->Reset());

	// Reset the command list, use empty pipeline state for now since there are no shaders and we are just clearing the screen.
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator, m_pipelineState));

	// Record commands in the command list now.
	// Start by setting the resource barrier.
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_backBufferRenderTarget[m_bufferIndex];
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	m_commandList->ResourceBarrier(1, &barrier);

	// Get the render target view handle for the current back buffer.
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle = m_renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
	unsigned int renderTargetViewDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	if (m_bufferIndex == 1)
		renderTargetViewHandle.ptr += renderTargetViewDescriptorSize;

	// Set the back buffer as the render target.
	m_commandList->OMSetRenderTargets(1, &renderTargetViewHandle, FALSE, NULL);

	// Then set the color to clear the window to.
	float color[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	m_commandList->ClearRenderTargetView(renderTargetViewHandle, color, 0, NULL);

	// Indicate that the back buffer will now be used to present.
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	m_commandList->ResourceBarrier(1, &barrier);

	// Close the list of commands.
	ThrowIfFailed(m_commandList->Close());

	// Load the command list array (only one command list for now).
	ID3D12CommandList* ppCommandLists[1];
	ppCommandLists[0] = m_commandList;

	// Execute the list of commands.
	m_commandQueue->ExecuteCommandLists(1, ppCommandLists);

	// Finally present the back buffer to the screen since rendering is complete.
	int syncInterval = m_vsyncEnabled ? 1 : 0;
	ThrowIfFailed(m_swapChain->Present(syncInterval, 0));

	// Signal and increment the fence value.
	unsigned long long fenceToWaitFor = m_fenceValue;
	ThrowIfFailed(m_commandQueue->Signal(m_fence, fenceToWaitFor));
	m_fenceValue++;

	// Wait until the GPU is done rendering.
	if (m_fence->GetCompletedValue() < fenceToWaitFor)
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(fenceToWaitFor, m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	// Alternate the back buffer index back and forth between 0 and 1 each frame.
	m_bufferIndex == 0 ? m_bufferIndex = 1 : m_bufferIndex = 0;
}
