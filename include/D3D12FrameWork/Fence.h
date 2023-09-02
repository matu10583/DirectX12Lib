#pragma once
#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/ComPtr.h"
#include <d3d12.h>

namespace D3D12FrameWork {
	class D3DDevice;
	class CommandQueue;
	class Fence
	{
	public:
		Fence();
		~Fence() {
			Term();
		}
		bool Init(D3DDevice* _pDev);

		void Signal(CommandQueue* _queue);
		void WaitSignal();
	private:
		void Term();

		ComPtr<ID3D12Fence> m_pFence;
		int32_t m_value;
		int32_t m_waitValue;
		HANDLE m_fenceEvent;

	};
}


