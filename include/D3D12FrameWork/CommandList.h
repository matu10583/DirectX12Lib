#pragma once
#include "D3D12FrameWork/ComPtr.h"
#include "D3D12FrameWork/TextureView.h"
#include "D3D12FrameWork/GlobalDescriptorHeap.h"
#include "D3D12FrameWork/Fence.h"
#include <initializer_list>
namespace D3D12FrameWork {
	class CommandQueue;
	class D3DDevice;
	class IResource;
	struct GlobalDescriptorHeapDesc;
	enum RPBufferType;
class CommandList
{
public:
	CommandList();
	~CommandList() {
		Term();
	}
	DECLMOVECOPY(CommandList);

	bool Init(D3DDevice* dev, D3D12_COMMAND_LIST_TYPE type,
		GlobalDescriptorHeapDesc const& _desc);
	bool Init(D3DDevice* dev, D3D12_COMMAND_LIST_TYPE type);
	void Term();

	void Begin();

	//Getter
	auto GetList()const {
		return m_pCommandList.Get();
	}
	auto GetAllocator()const {
		return m_pAllocator.Get();
	}

	void TransitState(IResource const* _resource,
		D3D12_RESOURCE_STATES _before,
		D3D12_RESOURCE_STATES _after);

	void SetAndClearRenderTargets(
		RenderTargetView const* _resViews,
		uint32_t numRenderTargets,
		//DsvHandle,
		std::initializer_list<float> _clearColor ={0.25f,0.25f,0.25f,1.0f}
	);

	void RegisterRootParameterLayout(
		uint32_t _rpIdx, RPBufferType _type, size_t numRegisters);
	void RegisterRootSignature(
		class RootSignature const* _rs
	);
	void SetGlobalDescriptorHeap();
	void Draw(class RenderComponent* _dc, D3DDevice* _pDev);
	void DrawMesh(class RenderComponent* _dc, D3DDevice* _pDev);
	void SetGraphicPipeline(
		class IPipelineStateObject const* _pso
	);

	void End();
	void Signal(CommandQueue* _queue) {
		m_fence.Signal(_queue);
	}
	void WaitSignal() {
		m_fence.WaitSignal();
	}

	void SetViewPort(D3D12_VIEWPORT const* const _vp, uint32_t _numVPs) {
		m_pCommandList->RSSetViewports(_numVPs,_vp);
	}
	void SetScissorRect(D3D12_RECT const* const _sr, uint32_t _numSRs) {
		m_pCommandList->RSSetScissorRects(_numSRs, _sr);
	}

private:
	ComPtr<ID3D12GraphicsCommandList6> m_pCommandList;
	ComPtr<ID3D12CommandAllocator> m_pAllocator;
	unqPtr<GlobalDescriptorHeap> m_rpHeap;
	Fence m_fence;
};

}
