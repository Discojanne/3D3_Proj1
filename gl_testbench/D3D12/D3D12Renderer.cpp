#include "D3D12Renderer.h"
#include "MaterialD3D12.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		exit(0);
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

template<class Interface>
inline void SafeRelease(
	Interface **ppInterfaceToRelease)
{
	if (*ppInterfaceToRelease != NULL)
	{
		(*ppInterfaceToRelease)->Release();

		(*ppInterfaceToRelease) = NULL;
	}
}


void SetResourceTransitionBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource,
	D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter)
{
	D3D12_RESOURCE_BARRIER barrierDesc = {};

	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Transition.pResource = resource;
	barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrierDesc.Transition.StateBefore = StateBefore;
	barrierDesc.Transition.StateAfter = StateAfter;

	commandList->ResourceBarrier(1, &barrierDesc);
}

D3D12Renderer::D3D12Renderer()
{
}

D3D12Renderer::~D3D12Renderer()
{
}

Material * D3D12Renderer::makeMaterial(const std::string & name)
{
	MaterialD3D12* mat = new MaterialD3D12(name);
	mat->device = mDevice5;
	return mat;
}

Mesh * D3D12Renderer::makeMesh()
{
	return nullptr;
}

VertexBuffer * D3D12Renderer::makeVertexBuffer(size_t size, VertexBuffer::DATA_USAGE usage)
{
	return nullptr;
}

ConstantBuffer * D3D12Renderer::makeConstantBuffer(std::string NAME, unsigned int location)
{
	return nullptr;
}

RenderState * D3D12Renderer::makeRenderState()
{
	return nullptr;
}

Technique * D3D12Renderer::makeTechnique(Material * m, RenderState * r)
{
	return nullptr;
}

Texture2D * D3D12Renderer::makeTexture2D()
{
	return nullptr;
}

Sampler2D * D3D12Renderer::makeSampler2D()
{
	return nullptr;
}

std::string D3D12Renderer::getShaderPath()
{
	return std::string("../assets/D3D12/");
}

std::string D3D12Renderer::getShaderExtension()
{
	return std::string(".hlsl");
}

int D3D12Renderer::initialize(unsigned int width, unsigned int height)
{

	HWND wndHandle = InitWindow(width, height);				//1. Create Window

	if (wndHandle)
	{
		if (!CreateDirect3DDevice(wndHandle))				//2. Create Device
		{
			return -2;
		}
		if (!CreateCommandInterfacesAndSwapChain(wndHandle))//3. Create CommandQueue and SwapChain
		{
			return -3;
		}
		if (!CreateFenceAndEventHandle())					//4. Create Fence and Event handle
		{
			return -4;
		}
		if (!CreateRenderTargets())							//5. Create render targets for backbuffer
		{
			return -5;
		}

		CreateViewportAndScissorRect(width, height);		//6. Create viewport and rect
			//return -6;

		if (!CreateRootSignature())							//7. Create root signature
		{
			return -7;
		}
		if (!CreateShadersAndPiplelineState())				//8. Set up the pipeline state
		{
			return -8;
		}


		if (!CreateConstantBufferResources())				//9. Create constant buffer data
			return -9;

		if (!CreateTriangleData()) //10. Create vertexdata
			return -10;


		WaitForGpu();

		ShowWindow(wndHandle, 1);
	}
	else
	{
		return -1;
	}

	//Constant buffer 
	ConstantBufferD3D12* cb_material = new ConstantBufferD3D12("Snopp", 0);
	DirectX::XMFLOAT4 matCol = {1.0f, 0.3f, 0.0f, 1.0f};

	cb_material->setData( &matCol, sizeof(DirectX::XMFLOAT4), nullptr, 0);

	ConstantBufferD3D12* cb_perObject[64];
	for (size_t i = 0; i < 64; i++)
	{
		cb_perObject[i] = new ConstantBufferD3D12("en", 1);
	}



//Start

	//Stuff below is just to test rendering. Remove it
	MSG msg = { 0 };
	float time = 0;

	while (WM_QUIT != msg.message)
	{
		time += 0.0001;

		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			UINT backBufferIndex = mSwapChain4->GetCurrentBackBufferIndex();

			for (int i = 0; i < 64; i++)
			{
				const DirectX::XMFLOAT4 trans{
					sin(time*0.5f + i / 10.0f) * 0.5f,
					cos(time*2.0f + i / 10.0f) * 0.5f,
					0.0f,
					0.0f
				};
				cb_perObject[i]->setData(&trans, sizeof(trans), nullptr, 2);
			}

			time += 0.0001;

			//Render

			//Command list allocators can only be reset when the associated command lists have
			//finished execution on the GPU; fences are used to ensure this (See WaitForGpu method)
			mCommandAllocator->Reset();
			mCommandList4->Reset(mCommandAllocator, mPipeLineState);
			
			//Set constant buffer descriptor heap
			ID3D12DescriptorHeap* descriptorHeaps[] = { mDescriptorHeap[backBufferIndex] };
			mCommandList4->SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);

			//Set root signature
			mCommandList4->SetGraphicsRootSignature(mRootSignature);

			//Set root descriptor table to index 0 in previously set root signature
			mCommandList4->SetGraphicsRootDescriptorTable(0,
				mDescriptorHeap[backBufferIndex]->GetGPUDescriptorHandleForHeapStart());

			//Set necessary states.
			mCommandList4->RSSetViewports(1, &mViewport);
			mCommandList4->RSSetScissorRects(1, &mScissorRect);

			//Indicate that the back buffer will be used as render target.
			SetResourceTransitionBarrier(mCommandList4,
				mRenderTargets[backBufferIndex],
				D3D12_RESOURCE_STATE_PRESENT,		//state before
				D3D12_RESOURCE_STATE_RENDER_TARGET	//state after
			);

			//Record commands.
			//Get the handle for the current render target used as back buffer.
			D3D12_CPU_DESCRIPTOR_HANDLE cdh = mRenderTargetsHeap->GetCPUDescriptorHandleForHeapStart();
			cdh.ptr += mRenderTargetDescriptorSize * backBufferIndex;

			mCommandList4->OMSetRenderTargets(1, &cdh, true, nullptr);

			float clearColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
			mCommandList4->ClearRenderTargetView(cdh, clearColor, 0, nullptr);

			mCommandList4->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			mCommandList4->IASetVertexBuffers(0, 1, &mVertexBufferView);


			//Update GPU memory
			void* mappedMem = nullptr;
			D3D12_RANGE readRange = { 0, 0 }; //We do not intend to read this resource on the CPU.

			if (SUCCEEDED(mConstantBufferResource[backBufferIndex]->Map(0, &readRange, &mappedMem)))
			{
				memcpy(mappedMem, cb_material->buff, sizeof(DirectX::XMFLOAT4));
				for (size_t i = 0; i < 64; i++)
				{
					memcpy(static_cast<void*>(static_cast<char*>(mappedMem) + cb_perObject[i]->size * i + sizeof(DirectX::XMFLOAT4)), cb_perObject[i]->buff, cb_perObject[i]->size);
				}

				D3D12_RANGE writeRange = { 0, sizeof(ConstantBuffer) };
				mConstantBufferResource[backBufferIndex]->Unmap(0, &writeRange);
			}

			mCommandList4->DrawInstanced(6, min((int)(time * 3), 64), 0, 0);
			
			//Indicate that the back buffer will now be used to present.
			SetResourceTransitionBarrier(mCommandList4,
				mRenderTargets[backBufferIndex],
				D3D12_RESOURCE_STATE_RENDER_TARGET,	//state before
				D3D12_RESOURCE_STATE_PRESENT		//state after
			);

			//Close the list to prepare it for execution.
			mCommandList4->Close();

			//Execute the command list.
			ID3D12CommandList* listsToExecute[] = { mCommandList4 };
			mCommandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

			//Present the frame.
			DXGI_PRESENT_PARAMETERS pp = {};
			mSwapChain4->Present1(0, 0, &pp);

			WaitForGpu(); //Wait for GPU to finish.
						  //NOT BEST PRACTICE, only used as such for simplicity.
		}
	}


	return 0;
}

void D3D12Renderer::setWinTitle(const char * title)
{
}

int D3D12Renderer::shutdown()
{
	return 0;
}

void D3D12Renderer::setClearColor(float, float, float, float)
{
}

void D3D12Renderer::clearBuffer(unsigned int)
{
}

void D3D12Renderer::setRenderState(RenderState * ps)
{
}

void D3D12Renderer::submit(Mesh* mesh)
{
}

void D3D12Renderer::frame()
{
}

void D3D12Renderer::present()
{
}

HWND D3D12Renderer::InitWindow(unsigned int width, unsigned int height)
{

	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = nullptr;
	wcex.lpszClassName = L"D3D12 Works!";
	if (!RegisterClassEx(&wcex))
	{
		return false;
	}

	RECT rc = { 0, 0, width, height };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

	return CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW,
		L"D3D12 Works!",
		L"D3D12 Works!",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		nullptr,
		nullptr);
}

void D3D12Renderer::WaitForGpu()
{
	//WAITING FOR EACH FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	//This is code implemented as such for simplicity. The cpu could for example be used
	//for other tasks to prepare the next frame while the current one is being rendered.

	//Signal and increment the fence value.
	const UINT64 fence = mFenceValue;
	mCommandQueue->Signal(mFence, fence);
	mFenceValue++;

	//Wait until command queue is done.
	if (mFence->GetCompletedValue() < fence)
	{
		mFence->SetEventOnCompletion(fence, mEventHandle);
		WaitForSingleObject(mEventHandle, INFINITE);
	}
}

bool D3D12Renderer::CreateDirect3DDevice(HWND wndHandle)
{
	//dxgi1_6 is only needed for the initialization process using the adapter.
	IDXGIFactory6*	factory = nullptr;
	IDXGIAdapter1*	adapter = nullptr;
	//First a factory is created to iterate through the adapters available.
	CreateDXGIFactory(IID_PPV_ARGS(&factory));
	for (UINT adapterIndex = 0;; ++adapterIndex)
	{
		adapter = nullptr;
		if (DXGI_ERROR_NOT_FOUND == factory->EnumAdapters1(adapterIndex, &adapter))
		{
			return false;	//No more adapters to enumerate.
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device5), nullptr)))
		{
			break;
		}

		SafeRelease(&adapter);
	}
	if (adapter)
	{
		HRESULT hr = S_OK;
		//Create the actual device.
		if (!SUCCEEDED(hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&mDevice5))))
		{
			return false;
		}

		SafeRelease(&adapter);
	}
	else
	{
		return false;
		////Create warp device if no adapter was found.
		//factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
		//D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice5));
	}

	SafeRelease(&factory);
	return true;
}

bool D3D12Renderer::CreateCommandInterfacesAndSwapChain(HWND wndHandle)
{
	HRESULT hr;
	//Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC cqd = {};
	hr = mDevice5->CreateCommandQueue(&cqd, IID_PPV_ARGS(&mCommandQueue));
	if (FAILED(hr))
	{
		return false;
	}

	//Create command allocator. The command allocator object corresponds
	//to the underlying allocations in which GPU commands are stored.
	hr = mDevice5->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCommandAllocator));
	if (FAILED(hr))
	{
		return false;
	}

	//Create command list.
	hr = mDevice5->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		mCommandAllocator,
		nullptr,
		IID_PPV_ARGS(&mCommandList4));
	if (FAILED(hr))
	{
		return false;
	}

	//Command lists are created in the recording state. Since there is nothing to
	//record right now and the main loop expects it to be closed, we close it.
	mCommandList4->Close();

	IDXGIFactory5*	factory = nullptr;
	hr = CreateDXGIFactory(IID_PPV_ARGS(&factory));
	if (FAILED(hr))
	{
		return false;
	}

	//Create swap chain.
	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	scDesc.Width = 0;
	scDesc.Height = 0;
	scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.Stereo = FALSE;
	scDesc.SampleDesc.Count = 1;
	scDesc.SampleDesc.Quality = 0;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = NUM_SWAP_BUFFERS;
	scDesc.Scaling = DXGI_SCALING_NONE;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.Flags = 0;
	scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	IDXGISwapChain1* swapChain1 = nullptr;

	hr = factory->CreateSwapChainForHwnd(
		mCommandQueue,
		wndHandle,
		&scDesc,
		nullptr,
		nullptr,
		&swapChain1);
	if (SUCCEEDED(hr))
	{
		hr = swapChain1->QueryInterface(IID_PPV_ARGS(&mSwapChain4));
		if (SUCCEEDED(hr))
		{
			mSwapChain4->Release();
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	SafeRelease(&factory);


	return true;
}

bool D3D12Renderer::CreateFenceAndEventHandle()
{
	HRESULT hr;
	hr = mDevice5->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));
	if (FAILED(hr))
	{
		return false;
	}

	mFenceValue = 1;
	//Create an event handle to use for GPU synchronization.
	mEventHandle = CreateEvent(0, false, false, 0);

	return true;
}

bool D3D12Renderer::CreateRenderTargets()
{
	//Create descriptor heap for render target views.
	D3D12_DESCRIPTOR_HEAP_DESC dhd = {};
	dhd.NumDescriptors = NUM_SWAP_BUFFERS;
	dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	HRESULT hr = mDevice5->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&mRenderTargetsHeap));
	if (FAILED(hr))
	{
		return false;
	}

	//Create resources for the render targets.
	mRenderTargetDescriptorSize = mDevice5->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = mRenderTargetsHeap->GetCPUDescriptorHandleForHeapStart();

	//One RTV for each frame.
	for (UINT n = 0; n < NUM_SWAP_BUFFERS; n++)
	{
		hr = mSwapChain4->GetBuffer(n, IID_PPV_ARGS(&mRenderTargets[n]));
		if (FAILED(hr))
		{
			return false;
		}

		mDevice5->CreateRenderTargetView(mRenderTargets[n], nullptr, cdh);
		cdh.ptr += mRenderTargetDescriptorSize;
	}

	return true;
}

void D3D12Renderer::CreateViewportAndScissorRect(unsigned int width, unsigned int height)
{
	mViewport.TopLeftX = 0.0f;
	mViewport.TopLeftY = 0.0f;
	mViewport.Width = (float)width;
	mViewport.Height = (float)height;
	mViewport.MinDepth = 0.0f;
	mViewport.MaxDepth = 1.0f;

	mScissorRect.left = (long)0;
	mScissorRect.right = (long)width;
	mScissorRect.top = (long)0;
	mScissorRect.bottom = (long)height;

}

bool D3D12Renderer::CreateShadersAndPiplelineState()
{
	HRESULT hr;

	////// Shader Compiles //////
	ID3DBlob* vertexBlob;
	hr = D3DCompileFromFile(
		L"../assets/D3D12/VertexShader_test.hlsl", // filename
		nullptr,		// optional macros
		nullptr,		// optional include files
		"main",		// entry point
		"vs_5_0",		// shader model (target)
		0,				// shader compile options			// here DEBUGGING OPTIONS
		0,				// effect compile options
		&vertexBlob,	// double pointer to ID3DBlob		
		nullptr			// pointer for Error Blob messages.
						// how to use the Error blob, see here
						// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
	);
	if (FAILED(hr))
	{
		return false;
	}

	ID3DBlob* pixelBlob;
	hr = D3DCompileFromFile(
		L"../assets/D3D12/FragmentShader_test.hlsl", // filename
		nullptr,		// optional macros
		nullptr,		// optional include files
		"main",		// entry point
		"ps_5_0",		// shader model (target)
		0,				// shader compile options			// here DEBUGGING OPTIONS
		0,				// effect compile options
		&pixelBlob,		// double pointer to ID3DBlob		
		nullptr			// pointer for Error Blob messages.
						// how to use the Error blob, see here
						// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
	);
	if (FAILED(hr))
	{
		return false;
	}

	////// Input Layout //////
	D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR"	, 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = inputElementDesc;
	inputLayoutDesc.NumElements = ARRAYSIZE(inputElementDesc);

	////// Pipline State //////
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsd = {};

	//Specify pipeline stages:
	gpsd.pRootSignature = mRootSignature;
	gpsd.InputLayout = inputLayoutDesc;
	gpsd.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpsd.VS.pShaderBytecode = reinterpret_cast<void*>(vertexBlob->GetBufferPointer());
	gpsd.VS.BytecodeLength = vertexBlob->GetBufferSize();
	gpsd.PS.pShaderBytecode = reinterpret_cast<void*>(pixelBlob->GetBufferPointer());
	gpsd.PS.BytecodeLength = pixelBlob->GetBufferSize();

	//Specify render target and depthstencil usage.
	gpsd.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpsd.NumRenderTargets = 1;

	gpsd.SampleDesc.Count = 1;
	gpsd.SampleMask = UINT_MAX;

	//Specify rasterizer behaviour.
	gpsd.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	gpsd.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;

	//Specify blend descriptions.
	D3D12_RENDER_TARGET_BLEND_DESC defaultRTdesc = {
		false, false,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL };
	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		gpsd.BlendState.RenderTarget[i] = defaultRTdesc;

	hr = mDevice5->CreateGraphicsPipelineState(&gpsd, IID_PPV_ARGS(&mPipeLineState));
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool D3D12Renderer::CreateTriangleData()
{
	HRESULT hr;
	Vertex triangleVertices[6] =
	{
		0.0f, 0.3f, 0.0f,	//v0 pos
		1.0f, 0.0f, 0.0f,	//v0 color

		0.3f, -0.3f, 0.0f,	//v1
		0.0f, 1.0f, 0.0f,	//v1 color

		-0.3f, -0.3f, 0.0f, //v2
		0.0f, 0.0f, 1.0f,	//v2 color

		0.0f, 0.3f, 0.0f,	//v0 pos
		1.0f, 0.0f, 0.0f,	//v0 color

		-0.3f, 0.6f, 0.0f,	//v1
		0.0f, 1.0f, 0.0f,	//v1 color

		0.3f, 0.6f, 0.0f, //v2
		0.0f, 0.0f, 1.0f	//v2 color
	};

	//Note: using upload heaps to transfer static data like vert buffers is not 
	//recommended. Every time the GPU needs it, the upload heap will be marshalled 
	//over. Please read up on Default Heap usage. An upload heap is used here for 
	//code simplicity and because there are very few vertices to actually transfer.
	D3D12_HEAP_PROPERTIES hp = {};
	hp.Type = D3D12_HEAP_TYPE_UPLOAD;
	hp.CreationNodeMask = 1;
	hp.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC rd = {};
	rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	rd.Width = sizeof(triangleVertices);
	rd.Height = 1;
	rd.DepthOrArraySize = 1;
	rd.MipLevels = 1;
	rd.SampleDesc.Count = 1;
	rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//Creates both a resource and an implicit heap, such that the heap is big enough
	//to contain the entire resource and the resource is mapped to the heap. 
	hr = mDevice5->CreateCommittedResource(
		&hp,
		D3D12_HEAP_FLAG_NONE,
		&rd,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&mVertexBufferResource));

	if (FAILED(hr))
		return false;

	mVertexBufferResource->SetName(L"vb heap");

	//Copy the triangle data to the vertex buffer.
	void* dataBegin = nullptr;
	D3D12_RANGE range = { 0, 0 }; //We do not intend to read this resource on the CPU.
	mVertexBufferResource->Map(0, &range, &dataBegin);
	memcpy(dataBegin, triangleVertices, sizeof(triangleVertices));
	mVertexBufferResource->Unmap(0, nullptr);

	//Initialize vertex buffer view, used in the render call.
	mVertexBufferView.BufferLocation = mVertexBufferResource->GetGPUVirtualAddress();
	mVertexBufferView.StrideInBytes = sizeof(Vertex);
	mVertexBufferView.SizeInBytes = sizeof(triangleVertices);

	return true;
}

bool D3D12Renderer::CreateRootSignature()
{
	HRESULT hr;

	//define descriptor range(s)
	D3D12_DESCRIPTOR_RANGE  dtRanges[1];
	dtRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	dtRanges[0].NumDescriptors = 1; //only one CB in this example
	dtRanges[0].BaseShaderRegister = 0; //register b0
	dtRanges[0].RegisterSpace = 0; //register(b0,space0);
	dtRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//create a descriptor table
	D3D12_ROOT_DESCRIPTOR_TABLE dt;
	dt.NumDescriptorRanges = ARRAYSIZE(dtRanges);
	dt.pDescriptorRanges = dtRanges;

	//create root parameter
	D3D12_ROOT_PARAMETER  rootParam[1];
	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[0].DescriptorTable = dt;
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	D3D12_ROOT_SIGNATURE_DESC rsDesc;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rsDesc.NumParameters = ARRAYSIZE(rootParam);
	rsDesc.pParameters = rootParam;
	rsDesc.NumStaticSamplers = 0;
	rsDesc.pStaticSamplers = nullptr;

	ID3DBlob* sBlob;
	hr = D3D12SerializeRootSignature(
		&rsDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&sBlob,
		nullptr);
	if (FAILED(hr))
	{
		return false;
	}

	hr = mDevice5->CreateRootSignature(
		0,
		sBlob->GetBufferPointer(),
		sBlob->GetBufferSize(),
		IID_PPV_ARGS(&mRootSignature));
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool D3D12Renderer::CreateConstantBufferResources()
{
	HRESULT hr;

	for (int i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDescriptorDesc = {};
		heapDescriptorDesc.NumDescriptors = 1;
		heapDescriptorDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDescriptorDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		hr = mDevice5->CreateDescriptorHeap(&heapDescriptorDesc, IID_PPV_ARGS(&mDescriptorHeap[i]));
		if (FAILED(hr))
			return false;
	}

	UINT cbSizeAligned = (sizeof(ConstantBuffer) + 255) & ~255;	// 256-byte aligned CB.

	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.CreationNodeMask = 1; //used when multi-gpu
	heapProperties.VisibleNodeMask = 1; //used when multi-gpu
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = cbSizeAligned;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//Create a resource heap, descriptor heap, and pointer to cbv for each frame
	for (int i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		hr = mDevice5->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&mConstantBufferResource[i])
		);
		if (FAILED(hr))
			return false;

		mConstantBufferResource[i]->SetName(L"cb heap");

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = mConstantBufferResource[i]->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = cbSizeAligned;
		mDevice5->CreateConstantBufferView(&cbvDesc, mDescriptorHeap[i]->GetCPUDescriptorHandleForHeapStart());
	}

	return true;
}
