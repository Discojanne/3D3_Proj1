#include "OpenGL/OpenGLRenderer.h"
#include "D3D12\D3D12Renderer.h"
#include "Renderer.h"


Renderer* Renderer::makeRenderer(BACKEND option)
{
	switch (option)
	{
	case Renderer::BACKEND::GL45:
		return new OpenGLRenderer();
		
	case Renderer::BACKEND::VULKAN:
		break;
	case Renderer::BACKEND::DX11:
		break;
	case Renderer::BACKEND::DX12:
		return new D3D12Renderer();
		
	default:
		break;
	}
	return nullptr;
}

