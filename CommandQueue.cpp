#include "D3D12FrameWork/CommandQueue.h"
#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/D3DDevice.h"

namespace D3D12FrameWork {
	CommandQueue::CommandQueue() {

	}

	void CommandQueue::Term() {

	}

	bool CommandQueue::Init(D3DDevice* dev, D3D12_COMMAND_LIST_TYPE type) {
		m_Type = type;
		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = type;
		desc.NodeMask = 0;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

		auto hr = dev->GetDev()->CreateCommandQueue(
			&desc,
			IID_PPV_ARGS(m_pCommandQueue.ReleaseAndGetAddressOf())
		);
		if (FAILED(hr)) {
			assert(SUCCEEDED(hr));
			return false;
		}
		return true;
	}
}