#include "D3D12FrameWork/CommandList.h"
#include "D3D12FrameWork/CommandQueue.h"
#include "D3D12FrameWork/D3DDevice.h"
#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/Texture.h"
#include "D3D12FrameWork/RootParameterBuffer.h"
#include "D3D12FrameWork/RenderComponent.h"
#include "D3D12FrameWork/PipelineStateObject.h"
#include "D3D12FrameWork/RootSignature.h"
namespace D3D12FrameWork {
	CommandList::CommandList() {

	}
	
	bool CommandList::Init(D3DDevice* dev, D3D12_COMMAND_LIST_TYPE _type,
		GlobalDescriptorHeapDesc const& _desc) {
		auto hr = dev->GetDev()->CreateCommandAllocator(
			_type,
			IID_PPV_ARGS(m_pAllocator.ReleaseAndGetAddressOf())
		);
		if (FAILED(hr)) {
			assert(SUCCEEDED(hr));
			return false;
		}

		hr = dev->GetDev()->CreateCommandList(
			0,
			_type,
			m_pAllocator.Get(),
			nullptr,
			IID_PPV_ARGS(m_pCommandList.ReleaseAndGetAddressOf())
		);
		if (FAILED(hr)) {
			assert(SUCCEEDED(hr));
			return false;
		}
		m_pCommandList->Close();
		
		m_rpHeap.reset(
			new GlobalDescriptorHeap()
		);
		return m_rpHeap->Init(dev, _desc);
	}

	bool CommandList::Init(D3DDevice* dev, D3D12_COMMAND_LIST_TYPE _type) {
		auto hr = dev->GetDev()->CreateCommandAllocator(
			_type,
			IID_PPV_ARGS(m_pAllocator.ReleaseAndGetAddressOf())
		);
		if (FAILED(hr)) {
			assert(SUCCEEDED(hr));
			return false;
		}

		hr = dev->GetDev()->CreateCommandList(
			0,
			_type,
			m_pAllocator.Get(),
			nullptr,
			IID_PPV_ARGS(m_pCommandList.ReleaseAndGetAddressOf())
		);
		if (FAILED(hr)) {
			assert(SUCCEEDED(hr));
			return false;
		}
		m_pCommandList->Close();
		return true;
	}

	void CommandList::Term() {
		
	}

	void CommandList::Begin() {
		m_pAllocator->Reset();
		m_pCommandList->Reset(m_pAllocator.Get(), nullptr);
	}

	void CommandList::TransitState(IResource const* _resource,
		D3D12_RESOURCE_STATES _before,
		D3D12_RESOURCE_STATES _after) {
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = _resource->GetResource();
		barrier.Transition.StateBefore = _before;
		barrier.Transition.StateAfter = _after;
		barrier.Transition.Subresource =
			D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		m_pCommandList->ResourceBarrier(1, &barrier);
	}

	void 
		CommandList::SetAndClearRenderTargets(
			RenderTargetView const* _resViews,
			uint32_t _numRenderTargets,
			//DsvHandle,
			std::initializer_list<float> _clearColor
		) {
		m_pCommandList->OMSetRenderTargets(
			_numRenderTargets,
			&_resViews->GetHeapInfo().m_CpuHandle,
			FALSE,
			nullptr
		);
		float col[4];
		int idx = 0;
		for (auto c:_clearColor) {
			col[idx] = c;
			idx++;
		}
		if (idx < 4) {
			for (int i = idx; i++; i < 4) {
				col[i] = 1.0f;
			}
		}



		m_pCommandList->ClearRenderTargetView(
			_resViews->GetHeapInfo().m_CpuHandle,
			col,
			0,
			nullptr
		);

	}

	void CommandList::End() {
		auto hr = m_pCommandList->Close();
		if (FAILED(hr)) {
			assert(false);
		}
	}
	
	void 
		CommandList::RegisterRootParameterLayout(
			uint32_t _rpIdx, RPBufferType _type, size_t numRegisters) {
		m_rpHeap->RegisterRPChankInfo(
			_rpIdx, _type, numRegisters
		);
	}

	void 
		CommandList::SetGlobalDescriptorHeap() {
		auto const heaps = m_rpHeap->GetHeaps();
		m_pCommandList->SetDescriptorHeaps(
			heaps.size(), heaps.data());
		for (int i = 0; i < m_rpHeap->GetRPChankNum(); i++) {
			m_pCommandList->SetGraphicsRootDescriptorTable(
				i, m_rpHeap->GetRPDescriptorHeapChank(i).GpuHandle
			);
		}
	}

	void
		CommandList::Draw(
			RenderComponent * _pRc, D3DDevice* _pDev
		) {
		bool isLayoutStable = true;

		//後でindexを使う際はマテリアル毎にメッシュを分けて描画するようにする。
		auto const matSize = _pRc->Material(0)->Size();
		for (int i = 0; i < matSize; i++) {
			auto const& matView = _pRc->Material(0)->GetView(i);
			//rootparameterにコピー
			isLayoutStable = isLayoutStable && m_rpHeap->CopyToHeapChank(_pDev,
				matView.GetCopyDesc(),
				i,
				matView.GetType()
			);
		}
		//rootparameterのセット
		if (!isLayoutStable) {
			SetGlobalDescriptorHeap();
		}

		//本当なら入る描画処理
		m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		auto const& meshView = _pRc->Mesh()->GetView();
		auto vbv = meshView.GetVBViews();
		m_pCommandList->IASetVertexBuffers(0, vbv.size(), vbv.data());

		m_pCommandList->DrawInstanced(meshView.NumVertices(), meshView.NumInstances(), 0, 0);

		_pRc->AfterDraw(0);
	}
	void CommandList::SetGraphicPipeline(IPipelineStateObject const* _pso) {
		m_pCommandList->SetPipelineState(_pso->GetPipelineState());
		m_pCommandList->SetGraphicsRootSignature(_pso->GetRootSignature()->GetDxRootSignature());
	}

	void CommandList::RegisterRootSignature(
		RootSignature const* _rs
	) {
		auto const& rpDesc = _rs->GetRootParamDescs();
		for (int i = 0; i < rpDesc.size(); i++) {
			size_t numDescs = 0;
			RPBufferType type;
			for (auto const& range : rpDesc[i].Ranges) {
				numDescs += range.NumDescriptors;
				switch (range.Type)
				{
				case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
				case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
				case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
					type = RPBufferType::CBV_SRV_UAV;
					break;
				case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
					type = RPBufferType::SMP;
					break;
				default:
					assert(false);
					break;
				}
			}

			m_rpHeap->RegisterRPChankInfo(
				i, type, numDescs
			);
		}
	}
}