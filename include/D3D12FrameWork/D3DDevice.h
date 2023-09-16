#pragma once
#include <dxgi1_6.h>
#include "D3D12FrameWork/ComPtr.h"
#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/DeathList.h"

namespace D3D12FrameWork {
	class CommandList;
	class CommandQueue;
	//class GlobalDescriptorPool;
	//class DescriptorPool;
	class DescriptorAllocator;
	class Fence;
	class D3DDevice
	{

	public:
		D3DDevice();
		~D3DDevice();
		DECLMOVECOPY(D3DDevice);

		bool Init(HWND const& _hwnd, LONG _width, LONG _height);
		void Term();

		template<typename T>
		void PendingRelease(T* pObj) {
			m_deathList.PendingRelease(pObj);
		}
		template<typename T>
		void PendingDelete(T* pObj) {
			m_deathList.PendingDelete(pObj);
		}
		void SyncKill() {
			m_deathList.SyncKill();
		}
		void EndAndExecuteCommandList(CommandList** _pCmdLists,
			uint32_t _numCmdLists);
		void WaitPresent();
		void Present(uint32_t _interval);


		//Getter
		auto GetDev()const {
			return m_pDevice.Get();
		}
		auto GetFactory()const {
			return m_pFactory.Get();
		}
		auto GetRTVHeaps()const {
			return m_pRtvHeap.get();
		}
		auto GetCbvSrvUavHeap()const {
			return m_pCbvSrvUavHeap.get();
		}
		auto GetDsvHeaps()const {
			return m_pDsvHeap.get();
		}
		auto GetSmpHeaps()const {
			return m_pSmpHeap.get();
		}
		/*auto GetCmdList()const {
			return m_pCommandLists.get();
		}*/
		auto GetCmdQueue()const {
			return m_pCommandQueue.get();
		}
		auto GetSwapChain()const {
			return m_pSwapChain.get();
		}

		bool CheckMeshShaderSupport();

	private:
		ComPtr<ID3D12Device8> m_pDevice;
		ComPtr<IDXGIFactory7> m_pFactory;
		//unqPtr<CommandList[]> m_pCommandLists;
		unqPtr<CommandQueue> m_pCommandQueue;
		unqPtr<SwapChain> m_pSwapChain;
		//https://sites.google.com/site/monshonosuana/directx%E3%81%AE%E8%A9%B1/directx%E3%81%AE%E8%A9%B1-%E7%AC%AC163%E5%9B%9E ÇéQçlÇ…Ç†Ç∆Ç≈Ç‚Ç¡ÇƒÇ›ÇÈ
		//unqPtr<GlobalDescriptorPool> m_pGlobalDescPool; 
		unqPtr<DescriptorAllocator> m_pCbvSrvUavHeap;
		unqPtr<DescriptorAllocator> m_pRtvHeap;
		unqPtr<DescriptorAllocator> m_pDsvHeap;
		unqPtr<DescriptorAllocator> m_pSmpHeap;
		DeathList m_deathList;
		//unqPtr<Fence> m_pFence;
		
	};
}

