#pragma once
#include "D3D12FrameWork/ComPtr.h"
#include <dxgi1_6.h>
#include <array>
#include"D3D12FrameWork/Common.h"
#include "D3D12FrameWork/Texture.h"
#include "D3D12FrameWork/DX12Settings.h"

namespace D3D12FrameWork {

	class D3DDevice;
	class CommandQueue;
	class RenderTargetView;
	class SwapChain
	{
	public:
		SwapChain();
		~SwapChain();

		bool Init(D3DDevice* dev,
			CommandQueue* queue,
			HWND hwnd,
			uint32_t w, uint32_t h);

		void Term();

		void Present(uint32_t _interval);
		void WaitPresent();


		auto GetDxSwapChain()const {
			return m_pSwapChain.Get();
		}
		auto GetCurrentFrameIndex()const {
			return m_frameIndex;
		}
		auto GetCurrentTexture()const {
			return m_textures[m_pSwapChain->GetCurrentBackBufferIndex()].get();
		}
		auto GetCurrentRTV()const {
			return m_rtvs[m_pSwapChain->GetCurrentBackBufferIndex()].get();
		}


	private:
		ComPtr<IDXGISwapChain4> m_pSwapChain;
		std::array<unqPtr<Texture>, DX12Settings::BUFFER_COUNT> m_textures;
		std::array<unqPtr<RenderTargetView>, DX12Settings::BUFFER_COUNT> m_rtvs;
		uint32_t m_frameIndex;
		HANDLE m_swapEvent;
	};

}
