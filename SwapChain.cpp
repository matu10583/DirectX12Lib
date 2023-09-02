#include "D3D12FrameWork/SwapChain.h"
#include "D3D12FrameWork/CommandQueue.h"
#include "D3D12FrameWork/D3DDevice.h"
#include "D3D12FrameWork/TextureView.h"

namespace D3D12FrameWork {
	SwapChain::SwapChain()
		:m_frameIndex(0){
	}
	
	bool SwapChain::Init(D3DDevice* dev,
		CommandQueue* queue,
		HWND hwnd,
		uint32_t w, uint32_t h) {
		//scの作製
		{
			DXGI_SWAP_CHAIN_DESC1 scDesc = {};
			scDesc.Width = w;
			scDesc.Height = h;
			scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			scDesc.Stereo = false;
			scDesc.SampleDesc.Count = 1;
			scDesc.SampleDesc.Quality = 0;
			scDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
			scDesc.BufferCount = DX12Settings::BUFFER_COUNT;
			scDesc.Scaling = DXGI_SCALING_STRETCH;
			scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
			scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

			IDXGISwapChain1* pSwap;
			auto hr = dev->GetFactory()->CreateSwapChainForHwnd(
				queue->GetQueue(),//queueのアドレスであることに注意
				hwnd,
				&scDesc,
				nullptr, nullptr,
				&pSwap
			);
			if (FAILED(hr)) {
				assert(SUCCEEDED(hr));
				return false;
			}

			//機能の拡張
			hr = pSwap->QueryInterface(
				IID_PPV_ARGS(m_pSwapChain.ReleaseAndGetAddressOf())
			);
			if (FAILED(hr)) {
				assert(SUCCEEDED(hr));
			}

			m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
			m_swapEvent = m_pSwapChain->GetFrameLatencyWaitableObject();

			pSwap->Release();
		}
		//テクスチャの初期化
		{
			for (int i = 0; i < DX12Settings::BUFFER_COUNT; i++) {
				m_textures[i] = std::make_unique<Texture>();
				auto result = m_textures[i]->InitFromSwapChain(dev, this, i);
				if (!result) return false;
				m_rtvs[i] = std::make_unique<RenderTargetView>();
				result = m_rtvs[i]->Init(dev, m_textures[i].get());
				if (!result) return false;
			}
		}
		return true;

	}

	SwapChain::~SwapChain() {
		Term();
	}

	void SwapChain::Term() {
		//for (int i = 0; i < BUFFER_COUNT(); i++) {
		//	m_rtvs[i].reset();
		//}
		//for (int i = 0; i < BUFFER_COUNT(); i++) {
		//	m_textures[i].reset();
		//}
		//m_pSwapChain->Release();
	}

	void SwapChain::Present(uint32_t _interval) {

		m_pSwapChain->Present(_interval, 0);
		m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

	}
	void SwapChain::WaitPresent() {
		auto result = WaitForSingleObjectEx(
			m_swapEvent,
			100,
			true
		);
		assert(SUCCEEDED(result));
	}
}