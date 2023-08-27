#pragma once
#include "D3D12FrameWork/ComPtr.h"
namespace D3D12FrameWork {

	class D3DDevice;
class CommandQueue
{
	friend class CommandList;
public:
	CommandQueue();
	~CommandQueue() {
		Term();
	}

	bool Init(D3DDevice* dev, D3D12_COMMAND_LIST_TYPE type);
	void Term();

	//Getter
	auto GetQueue()const {
		return m_pCommandQueue.Get();
	}
	auto const GetType()const { return m_Type; }
private:
	ComPtr<ID3D12CommandQueue> m_pCommandQueue;
	D3D12_COMMAND_LIST_TYPE m_Type;
};

}
