#pragma once
#include "../Material.h"
#include <vector>
#include "ConstantBufferD3D12.h"
#include <d3d12.h>
#pragma comment (lib, "d3dcompiler.lib")

//#include "ConstantBufferGL.h"

class D3D12Renderer;

#define DBOUTW( s )\
{\
std::wostringstream os_;\
os_ << s;\
OutputDebugStringW( os_.str().c_str() );\
}

#define DBOUT( s )\
{\
std::ostringstream os_;\
os_ << s;\
OutputDebugString( os_.str().c_str() );\
}

// use X = {Program or Shader}
#define INFO_OUT(S,X) { \
char buff[1024];\
memset(buff, 0, 1024);\
glGet##X##InfoLog(S, 1024, nullptr, buff);\
DBOUTW(buff);\
}

// use X = {Program or Shader}
#define COMPILE_LOG(S,X,OUT) { \
char buff[1024];\
memset(buff, 0, 1024);\
glGet##X##InfoLog(S, 1024, nullptr, buff);\
OUT=std::string(buff);\
}


class MaterialD3D12 :
	public Material
{
	friend D3D12Renderer;

public:
	MaterialD3D12(const std::string& name);
	~MaterialD3D12();


	void setShader(const std::string& shaderFileName, ShaderType type);
	void removeShader(ShaderType type);
	int compileMaterial(std::string& errString);
	int enable();
	void disable();
	unsigned int getProgram() { return 0; };
	void setDiffuse(Color c);

	// location identifies the constant buffer in a unique way
	void updateConstantBuffer(const void* data, size_t size, unsigned int location);
	// slower version using a string
	void addConstantBuffer(std::string name, unsigned int location);
	std::map<unsigned int, ConstantBufferD3D12*> constantBuffers;

private:
	// map from ShaderType to GL_VERTEX_SHADER, should be static.
	unsigned int mapShaderEnum[4];

	std::string shaderNames[4];

	// D3D12 shader Blobs
	ID3DBlob* shaderBlob[4];

	// TODO: change to PIPELINE
	// opengl program object
	std::string name;


	ID3D12Device5*			device			= nullptr;
	ID3D12RootSignature*	mRootSignature	= nullptr;

	ID3D12PipelineState*	mPipeLineState	= nullptr;

	//unsigned int program;
	int compileShader(ShaderType type, std::string& errString);
	std::vector<std::string> expandShaderText(std::string& shaderText, ShaderType type);
	void setDevice(ID3D12Device5* pdevice5);
	void setRootSignature(ID3D12RootSignature* RootSignature);

};

