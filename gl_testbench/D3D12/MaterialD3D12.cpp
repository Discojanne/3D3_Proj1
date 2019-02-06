#include <windows.h>
#include <streambuf>
#include <sstream>
#include <istream>
#include <fstream>
#include <vector>
#include <set>
#include <assert.h>
#include <d3dcompiler.h>
#include <string>
#include "Helper.hpp"
#include "MaterialD3D12.h"

#include <dxgi1_6.h> //Only used for initialization of the device and swap chain.

typedef unsigned int uint;


MaterialD3D12::MaterialD3D12(const std::string & name)
{
	
}

MaterialD3D12::~MaterialD3D12()
{
}

void MaterialD3D12::setShader(const std::string & shaderFileName, ShaderType type)
{
	if (shaderFileNames.find(type) != shaderFileNames.end())
	{
		removeShader(type);
	}
	shaderFileNames[type] = shaderFileName;
}

void MaterialD3D12::removeShader(ShaderType type)
{
}

int MaterialD3D12::compileMaterial(std::string & errString)
{
	HRESULT hr;

	for (auto shader : shaderFileNames) {

		ShaderType type = shader.first;
		std::string name = shader.second;
		std::wstring wname = towstr(name);
		std::string model;

		switch (type)
		{
		case Material::ShaderType::VS:
			model = "vs_5_0";
			break;
		case Material::ShaderType::PS:
			model = "ps_5_0";
			break;
		case Material::ShaderType::GS:
			model = "gs_5_0";
			break;
		case Material::ShaderType::CS:
			model = "cs_5_0";
			break;
		default:
			break;
		}

		////// Shader Compiles //////
		hr = D3DCompileFromFile(
			wname.c_str(), // filename
			nullptr,		// optional macros
			nullptr,		// optional include files
			"main",		// entry point
			model.c_str(),		// shader model (target)
			0,				// shader compile options			// here DEBUGGING OPTIONS
			0,				// effect compile options
			&shaderBlob[static_cast<int>(type)],	// double pointer to ID3DBlob		
			nullptr			// pointer for Error Blob messages.
							// how to use the Error blob, see here
							// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
		);
		if (FAILED(hr))
		{
			return false;
		}
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
	gpsd.VS.pShaderBytecode = reinterpret_cast<void*>(shaderBlob[static_cast<int>(ShaderType::VS)]->GetBufferPointer());
	gpsd.VS.BytecodeLength = shaderBlob[static_cast<int>(ShaderType::VS)]->GetBufferSize();
	gpsd.PS.pShaderBytecode = reinterpret_cast<void*>(shaderBlob[static_cast<int>(ShaderType::PS)]->GetBufferPointer());
	gpsd.PS.BytecodeLength = shaderBlob[static_cast<int>(ShaderType::PS)]->GetBufferSize();

	//Specify render target and depthstencil usage.
	gpsd.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpsd.NumRenderTargets = 1;

	gpsd.SampleDesc.Count = 1;
	gpsd.SampleMask = UINT_MAX;

	//Specify rasterizer behaviour.
	gpsd.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	gpsd.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;

	//Specify blend descriptions.
	D3D12_RENDER_TARGET_BLEND_DESC defaultRTdesc = {
		false, false,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL };
	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		gpsd.BlendState.RenderTarget[i] = defaultRTdesc;

	hr = device->CreateGraphicsPipelineState(&gpsd, IID_PPV_ARGS(&mPipeLineState));
	if (FAILED(hr))
	{
		return false;
	}
	return 0;
}

int MaterialD3D12::enable()
{
	return 0;
}

void MaterialD3D12::disable()
{
}

void MaterialD3D12::setDiffuse(Color c)
{
}

void MaterialD3D12::updateConstantBuffer(const void * data, size_t size, unsigned int location)
{
}

void MaterialD3D12::addConstantBuffer(std::string name, unsigned int location)
{
}

int MaterialD3D12::compileShader(ShaderType type, std::string & errString)
{
	return 0;
}

std::vector<std::string> MaterialD3D12::expandShaderText(std::string & shaderText, ShaderType type)
{
	return std::vector<std::string>();
}

void MaterialD3D12::setDevice(ID3D12Device5 * pdevice5)
{
	device = pdevice5;
}

void MaterialD3D12::setRootSignature(ID3D12RootSignature * RootSignature)
{
	mRootSignature = RootSignature;
}
