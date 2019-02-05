#include "ConstantBufferD3D12.h"
//#include "MaterialGL.h"

ConstantBufferD3D12::ConstantBufferD3D12(std::string NAME, unsigned int location)
{
	this->name = NAME;
}

ConstantBufferD3D12::~ConstantBufferD3D12()
{
}

void ConstantBufferD3D12::setData(const void * data, size_t size, Material * m, unsigned int location)
{
	if (!buff){
		buff = new char[size];
		buffSize = size;
	}
	else if (buffSize < size) {
		delete[] buff;
		buff = nullptr;
		buffSize = size;
	}

	memcpy(buff, data, size);
}

void ConstantBufferD3D12::bind(Material *)
{
}
