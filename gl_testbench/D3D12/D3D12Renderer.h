#pragma once

#include "../Renderer.h"
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h> //Only used for initialization of the device and swap chain.
#include <d3dcompiler.h>
#include <DirectXMath.h>

#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "DXGI.lib")
//#pragma comment (lib, "d3dcompiler.lib")

struct Vertex
{
	float x, y, z; // Position
	float r, g, b; // Color
};

class D3D12Renderer : public Renderer
{
public:
	D3D12Renderer();
	~D3D12Renderer();

	Material* makeMaterial(const std::string& name);
	Mesh* makeMesh();
	//VertexBuffer* makeVertexBuffer();
	VertexBuffer* makeVertexBuffer(size_t size, VertexBuffer::DATA_USAGE usage);
	ConstantBuffer* makeConstantBuffer(std::string NAME, unsigned int location);
//	ResourceBinding* makeResourceBinding();
	RenderState* makeRenderState();
	Technique* makeTechnique(Material* m, RenderState* r);
	Texture2D* makeTexture2D();
	Sampler2D* makeSampler2D();
	std::string getShaderPath();
	std::string getShaderExtension();

	int initialize(unsigned int width = 640, unsigned int height = 480);
	void setWinTitle(const char* title);
	int shutdown();

	void setClearColor(float, float, float, float);
	void clearBuffer(unsigned int);
//	void setRenderTarget(RenderTarget* rt); // complete parameters
	void setRenderState(RenderState* ps);
	void submit(Mesh* mesh);
	void frame();
	void present();

private:
	HWND InitWindow(unsigned int width, unsigned int height);
	
	//Helper function for syncronization of GPU/CPU
	void WaitForGpu();
	//Helper function for resource transitions

	bool CreateDirect3DDevice(HWND wndHandle);					//2. Create Device
	bool CreateCommandInterfacesAndSwapChain(HWND wndHandle);	//3. Create CommandQueue and SwapChain
	bool CreateFenceAndEventHandle();							//4. Create Fence and Event handle
	bool CreateRenderTargets();									//5. Create render targets for backbuffer
	void CreateViewportAndScissorRect(unsigned int width, unsigned int height);//6. Create viewport and rect
	bool CreateShadersAndPiplelineState();						//7. Set up the pipeline state
	bool CreateTriangleData();									//8. Create vertexdata
	bool CreateRootSignature();
	bool CreateConstantBufferResources(int location);

	static const unsigned int NUM_SWAP_BUFFERS = 2; //Number of buffers

	std::unordered_map<Technique*, std::vector<Mesh*>> mDrawList;

	ID3D12Device5*				mDevice5 = nullptr;

	ID3D12CommandQueue*			mCommandQueue = nullptr;
	ID3D12CommandAllocator*		mCommandAllocator = nullptr;
	IDXGISwapChain4*			mSwapChain4 = nullptr;
	ID3D12GraphicsCommandList4*	mCommandList4 = nullptr;

	ID3D12Fence1*				mFence = nullptr;
	HANDLE						mEventHandle = nullptr;
	UINT64						mFenceValue = 0;

	ID3D12DescriptorHeap*		mRenderTargetsHeap = nullptr;
	ID3D12Resource1*			mRenderTargets[NUM_SWAP_BUFFERS] = {};
	UINT						mRenderTargetDescriptorSize = 0;

	D3D12_VIEWPORT				mViewport = {};
	D3D12_RECT					mScissorRect = {};

	ID3D12RootSignature*		mRootSignature = nullptr;
	//ID3D12PipelineState*		mPipeLineState = nullptr;

	ID3D12Resource1*			mVertexBufferResource = nullptr;
	D3D12_VERTEX_BUFFER_VIEW	mVertexBufferView = {};

	//struct ConstantBuffer
	//{
	//	DirectX::XMFLOAT4X4 world[32];
	//	DirectX::XMFLOAT4 color[32];
	//};

	//ID3D12DescriptorHeap*	mDescriptorHeap[NUM_SWAP_BUFFERS] = {};
	//ID3D12Resource1*		mConstantBufferResource[NUM_SWAP_BUFFERS] = {};
	//ConstantBuffer			mConstantBufferCPU = {};

	struct CB_Struct{};

	struct ConstantBuffer_Translation : public CB_Struct {
		DirectX::XMFLOAT4 translation[32];
	};

	struct ConstantBuffer_Diffuse_Tint : public CB_Struct {
		DirectX::XMFLOAT4 diffuse[32];
	};

	std::unordered_map<int, std::unordered_map<int, ID3D12DescriptorHeap*>> mDescriptorHeaps;
	std::unordered_map<int, std::unordered_map<int, ID3D12Resource1*>>		mConstantBufferResources;
	std::unordered_map<int, std::unordered_map<int, CB_Struct*>>		mConstantBuffers;


};

