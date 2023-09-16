#include "D3D12FrameWork/Fence.h"
#include "D3D12FrameWork/D3DDevice.h"
#include "D3D12FrameWork/CommandQueue.h"
#include <comdef.h>

namespace D3D12FrameWork {
	Fence::Fence()
		:m_pFence(nullptr)
		, m_value(1)
		, m_waitValue(0)
		//, m_fenceEvent(nullptr) 
	{

	}

	bool Fence::Init(D3DDevice* _pDev) {
		auto hr = _pDev->GetDev()->CreateFence(
			m_waitValue,
			D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(m_pFence.ReleaseAndGetAddressOf())
		);
		if (FAILED(hr)) {
			assert(false);
			return false;
		}
		//m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	/*	if (m_fenceEvent == nullptr) {
			assert(false);
		}*/

		return true;
	}

	void Fence::Signal(CommandQueue* _queue) {
		m_waitValue = m_value;

		auto hr = _queue->GetQueue()->Signal(m_pFence.Get(), m_waitValue);
		if (FAILED(hr)) assert(false);
		m_value++;

	}

	void Fence::WaitSignal() {
		if (m_pFence->GetCompletedValue() < m_waitValue) {
			//fence eventでエラーが起こる。わかんない。。。
			auto hr = m_pFence->SetEventOnCompletion(m_waitValue, NULL);
			if (FAILED(hr)) assert(false);
			//WaitForSingleObject(m_fenceEvent, INFINITE);
		}
	}

	void Fence::Term() {
		//if (!CloseHandle(m_fenceEvent)) {
		//	assert(false);
		//}
	}
}


