#include "D3D12FrameWork/D3DDevice.h"
#include "D3D12FrameWork/CommandQueue.h"
#include "D3D12FrameWork/CommandList.h"
#include "D3D12FrameWork/SwapChain.h"
#include "D3D12FrameWork/Fence.h"
#include "D3D12FrameWork/BufferView.h"
#include "D3D12FrameWork/TextureView.h"
#include "D3D12FrameWork/SamplerView.h"
#include <vector>

namespace D3D12FrameWork {
	D3DDevice::D3DDevice() {

	}
	D3DDevice::~D3DDevice() {
		Term();
	}

	bool D3DDevice::Init(HWND const& _hwnd, LONG _width,LONG _height) {
		uint32_t factoryFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		//デバッグレイヤーの有効か
		ID3D12Debug* debug;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)))) {
			debug->EnableDebugLayer();
			debug->Release();
			debug = nullptr;
		}
#endif // defined(DEBUG) || defined(_DEBUG)

		auto hr = CreateDXGIFactory2(0, IID_PPV_ARGS(m_pFactory.ReleaseAndGetAddressOf()));
		if (FAILED(hr)) {
			assert(SUCCEEDED(hr));
			return false;
		}

		hr = D3D12CreateDevice(
			nullptr,
			D3D_FEATURE_LEVEL_11_0,//デバイスによってチェックする必要あり
			IID_PPV_ARGS(m_pDevice.ReleaseAndGetAddressOf())
		);
		if (FAILED(hr)) {
			assert(SUCCEEDED(hr));
			return false;
		}

		//ディスクリプタヒープの作製
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NodeMask = 0;
		
		// //CBV,SRV,UAV
		{
			heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			heapDesc.NumDescriptors = 100;
			m_pCbvSrvUavHeap =std::make_unique<DescriptorAllocator>();
			if (!m_pCbvSrvUavHeap->Init(
				this,
				heapDesc
			)) {
				assert(false);
				return false;
			}
		}
		// //RTV
		{
			heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			heapDesc.NumDescriptors = 100;
			m_pRtvHeap = std::make_unique<DescriptorAllocator>();
			if (!m_pRtvHeap->Init(
				this,
				heapDesc
			)) {
				assert(false);
				return false;
			}
		}
		// Dsv
		{
			heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			heapDesc.NumDescriptors = 20;
			m_pDsvHeap = std::make_unique<DescriptorAllocator>();
			if (!m_pDsvHeap->Init(
				this,
				heapDesc
			)) {
				assert(false);
				return false;
			}
		}
		// Smp
		{
			heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
			heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			heapDesc.NumDescriptors = 20;
			m_pSmpHeap = std::make_unique<DescriptorAllocator>();
			if (!m_pSmpHeap->Init(
				this,
				heapDesc
			)) {
				assert(false);
				return false;
			}
		}

		m_pCommandQueue = std::make_unique<CommandQueue>();
		if (!m_pCommandQueue->Init(this, D3D12_COMMAND_LIST_TYPE_DIRECT)) {
			assert(false);
			return false;
		}
		

		m_pSwapChain = std::make_unique<SwapChain>();
		if (!m_pSwapChain->Init(this, m_pCommandQueue.get(), _hwnd,_width, _height)) {
			assert(false);
			return false;
		}

		////fence
		//m_pFence = std::make_unique<Fence>();
		//if (!m_pFence->Init(this)) {
		//	assert(false);
		//	return false;
		//}
		
		if (!ConstantBufferView::CreateNullView(this)) {
			assert(false);
			return false;
		}
		if (!ShaderResourceView::CreateNullView(this)) {
			assert(false);
			return false;
		}
		if (!SamplerView::CreateNullView(this)) {
			assert(false);
			return false;
		}
		return true;
	}

	void 
		D3DDevice::WaitPresent() {
		if (m_pSwapChain.get()) m_pSwapChain.get()->WaitPresent();
	}

	void
		D3DDevice::Term() {
		//m_pFence->WaitSignal();
		WaitPresent();
		//順番に破棄する必要がある．

		m_deathList.SyncKill();

		ShaderResourceView::DestroyNullView();
		ConstantBufferView::DestroyNullView();
		SamplerView::DestroyNullView();

		//m_pFence.reset();
		//delete graphic systems
		m_pSwapChain.reset();
		m_pCommandQueue.reset();

		//delete Heaps
		m_pDsvHeap.reset();
		m_pRtvHeap.reset();
		m_pCbvSrvUavHeap.reset();

		//delete dx12Systems
		m_pDevice.Reset();
		m_pFactory.Reset();
	}

	void
		D3DDevice::EndAndExecuteCommandList(CommandList** _cmdLists, uint32_t _numCmdLists) {
		std::vector<ID3D12CommandList*> pCmdLists;
		pCmdLists.resize(_numCmdLists);
		for (int i = 0; i < _numCmdLists;i++) {
			_cmdLists[i]->End();
			pCmdLists[i] = _cmdLists[i]->GetList();
		}
		m_pCommandQueue->GetQueue()->ExecuteCommandLists(
			_numCmdLists,
			pCmdLists.data()
		);
		for (int i = 0; i < _numCmdLists; i++) {
			_cmdLists[i]->Signal(m_pCommandQueue.get());
		}
	}
	void 
		D3DDevice::Present(uint32_t _interval) {
		m_pSwapChain->Present(_interval);

		//m_pFence->Signal(m_pCommandQueue.get());
		//m_pFence->WaitSignal();
	}


	bool D3DDevice::CheckMeshShaderSupport() {
			{
				//shade model をチェック
				D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = {
					D3D_SHADER_MODEL_6_5
				};
				auto hr = m_pDevice->CheckFeatureSupport(
					D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)
				);
				if (FAILED(hr) ||
					shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_5) {
					OutputDebugStringA("Error: Shader Model 6.5 is not supported.");
					return false;
				}
			}

			//msをチェック
			{
				D3D12_FEATURE_DATA_D3D12_OPTIONS7 features = {};
				auto hr = m_pDevice->CheckFeatureSupport(
					D3D12_FEATURE_D3D12_OPTIONS7, &features,
					sizeof(features)
				);
				if (FAILED(hr) ||
					features.MeshShaderTier == D3D12_MESH_SHADER_TIER_NOT_SUPPORTED) {
					OutputDebugStringA("Error: Mesh Shaders aren't supported");
					return false;
				}
			}
			return true;
	}
}