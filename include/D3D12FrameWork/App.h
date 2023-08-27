#pragma once
#include <cstdint>
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include "D3D12FrameWork/ComPtr.h"
#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/D3DDevice.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "DirectXTex.lib")
namespace D3D12FrameWork {
	class App
	{
	public:
		App(uint32_t width, uint32_t height)
			:m_width(width)
			,m_height(height)
			, m_hInst(nullptr)
			, m_hwnd(nullptr) {}
		virtual ~App(){}
		virtual bool Init()=0;
		virtual void Term()=0;
		virtual void MainLoop()=0;
	protected:
		D3DDevice m_device;

		HINSTANCE m_hInst;
		HWND m_hwnd;
		LONG m_width;
		LONG m_height;
	};
}


