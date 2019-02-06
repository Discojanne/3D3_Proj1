#pragma once
#include "../ConstantBuffer.h"
#include "D3D12Renderer.h"

class ConstantBufferD3D12 : public ConstantBuffer
{
public:
	ConstantBufferD3D12(std::string NAME, unsigned int location);
	~ConstantBufferD3D12();
	void setData(const void* data, size_t size, Material* m, unsigned int location);
	void bind(Material*);

private:
	friend D3D12Renderer;
	//std::string name;
	//unsigned int location;
	//unsigned int handle;
	//unsigned int index;
	void* buff = nullptr;
	int size;
	//void* lastMat;
};

