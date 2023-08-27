#include "D3D12FrameWork/DescriptorAllocator.h"
#include "D3D12FrameWork/D3DDevice.h"
#include "D3D12FrameWork/Common.h"

namespace D3D12FrameWork{
	void DescriptorInfo::Free() {
		if (m_pAllocator == nullptr) {
			return;
		}

		m_pAllocator->Free(*this);
		m_pAllocator = nullptr;
		m_pItem = nullptr;
	}


	DescriptorAllocator::DescriptorAllocator()
		:m_pBuffer(nullptr)
		, m_pFreeList(nullptr)
		, m_IncrementSize(0)
		, m_pHeap(nullptr)
		, m_HeapDesc() {}


	bool DescriptorAllocator::Init(D3DDevice* dev
		, D3D12_DESCRIPTOR_HEAP_DESC const& desc) {


		m_HeapDesc = desc;

		
		//ヒープの作製
		auto hr = dev->GetDev()->CreateDescriptorHeap(
			&desc, IID_PPV_ARGS(m_pHeap.ReleaseAndGetAddressOf())
		);
		if (FAILED(hr)) {
			assert(SUCCEEDED(hr));
			return false;
		}
		m_IncrementSize = dev->GetDev()->GetDescriptorHandleIncrementSize(desc.Type);
		m_hCpuHead = m_pHeap->GetCPUDescriptorHandleForHeapStart();
		m_hGpuHead = m_pHeap->GetGPUDescriptorHandleForHeapStart();

		//メモリ管理情報の作製
		m_pBuffer = std::make_unique<uint8_t[]>(
			desc.NumDescriptors * sizeof(DescriptorInfo::Item)
			);
			//static_cast<uint8_t*>(
			//std::malloc(desc.NumDescriptors * sizeof(DescriptorInfo::Item)));
		m_pFreeList = GetItem(0);
		m_pFreeList->m_Index = 0;
		m_pFreeList->m_Next = GetItem(1);
		for (auto i = 1u; i < desc.NumDescriptors-1; i++) {
			auto item = GetItem(i);
			item->m_Index = i;
			item->m_Next = GetItem(i + 1);
		}
		auto tmpItem = GetItem(desc.NumDescriptors - 1);
		tmpItem->m_Index = desc.NumDescriptors - 1;
		tmpItem->m_Next = nullptr;

		return true;
	}

	DescriptorInfo::Item* DescriptorAllocator::GetItem(uint32_t index) {
		assert(index < m_HeapDesc.NumDescriptors);
		return reinterpret_cast<DescriptorInfo::Item*>(
			m_pBuffer.get() + sizeof(DescriptorInfo::Item) * index
			);
	}

	DescriptorInfo const& DescriptorAllocator::Allocate() {
		std::lock_guard<std::mutex> lockguard(m_mutex);
		DescriptorInfo retInfo = {};
		if (m_pFreeList == nullptr) {
			LOG_ERR("Memory Pool Shortage!");
			return retInfo;
		}
		//空いているメモリを割り当て
		auto idx = m_pFreeList->m_Index;
		retInfo.m_pItem = m_pFreeList;
		retInfo.m_CpuHandle = m_hCpuHead;
		retInfo.m_CpuHandle.ptr += m_IncrementSize * idx;
		retInfo.m_GpuHandle = m_hGpuHead;
		retInfo.m_GpuHandle.ptr += m_IncrementSize * idx;
		retInfo.m_pAllocator = this;

		//空き情報更新
		m_pFreeList = m_pFreeList->m_Next;

		return retInfo;
	}

	void DescriptorAllocator::Free(DescriptorInfo const& descInfo) {
		std::lock_guard<std::mutex> lockguard(m_mutex);

		if (descInfo.m_pItem == nullptr) {
			assert(false);
			return;
		}
		//空き情報更新
		descInfo.m_pItem->m_Next = m_pFreeList;
		m_pFreeList = descInfo.m_pItem;
	}

	void DescriptorAllocator::Term() {

	}
}
