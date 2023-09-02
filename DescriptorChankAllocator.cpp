#include "D3D12FrameWork/DescriptorChankAllocator.h"
#include "D3D12FrameWork/D3DDevice.h"
#include <stdexcept>


namespace D3D12FrameWork{
	DescriptorChankAllocator::DescriptorChankAllocator()
		:m_hCpuHead()
		,m_hGpuHead()
		,m_hItemEnd(0)
		,m_IncrementSize(0)
		,m_itemList()
		,m_pHeap(nullptr){}
	DescriptorChankAllocator::~DescriptorChankAllocator(){}
	bool
		DescriptorChankAllocator::Init(
			D3DDevice* _pDev,
			D3D12_DESCRIPTOR_HEAP_DESC const& _desc
		) {
		if (_pDev == nullptr) {
			return false;
		}
		m_numDesc = _desc.NumDescriptors;
		m_type = _desc.Type;
		auto hr = _pDev->GetDev()->CreateDescriptorHeap(
			&_desc,
			IID_PPV_ARGS(m_pHeap.ReleaseAndGetAddressOf())
		);
		if (FAILED(hr)) {
			assert(false);
			return false;
		}
		m_IncrementSize = _pDev->GetDev()->GetDescriptorHandleIncrementSize(
			_desc.Type
		);
		m_hCpuHead = m_pHeap->GetCPUDescriptorHandleForHeapStart();
		m_hGpuHead = m_pHeap->GetGPUDescriptorHandleForHeapStart();

		m_hItemEnd = _desc.NumDescriptors * m_IncrementSize;

		DescriptorChankInfo::Item tmp;
		tmp.m_ItemHandleForStart = 0;
		tmp.m_isAllocated = false;
		tmp.m_isBegin = true;
		tmp.m_isEnd = true;
		m_itemList.emplace_back(tmp);

		return true;
	}

	DescriptorChankInfo const
		DescriptorChankAllocator::Allocate(uint32_t _numDesc) {
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_itemList.empty()) {
			throw new std::runtime_error("GlobalDescriptorのメモリ領域が足りないです．動的メモリ確保を検討しろ");
		}
		auto const allocSize = _numDesc * m_IncrementSize;

		ItemList::iterator itAllocedItem;
		bool canAllocate = false;
		for (auto it = m_itemList.begin(); it != m_itemList.end();it++) {
			if (!it->m_isAllocated &&
				GetSize(it) >= allocSize) {//入る．
				itAllocedItem = it;
				canAllocate = true;
				break;
			}
		}
		if (!canAllocate) {//確保できまつぇん；；
			throw new std::runtime_error("GlobalDescriptorのメモリ領域が足りないです．動的メモリ確保を検討しろ");
		}

		//確保処理．
		DescriptorChankInfo ret;
		ret.NumDescriptors = _numDesc;
		ret.pAllocator = this;
		ret.itItem = itAllocedItem;
		ret.CpuHandle = m_hCpuHead;
		ret.CpuHandle.ptr += itAllocedItem->m_ItemHandleForStart;
		ret.GpuHandle = m_hGpuHead;
		ret.GpuHandle.ptr += itAllocedItem->m_ItemHandleForStart;

		//Itemリストを整理
		itAllocedItem->m_isAllocated = true;
		if (allocSize == GetSize(itAllocedItem)) {//ぴったりフリーリストに収まった
			return ret;
		}
		//一部だけ占有しているのでもう一つフラグメントが増える．
		DescriptorChankInfo::Item newItem;
		uint32_t newHandle = itAllocedItem->m_ItemHandleForStart +
			m_IncrementSize * _numDesc;
		newItem.m_isAllocated = false;
		newItem.m_ItemHandleForStart = newHandle;
		newItem.m_itNext = itAllocedItem->m_itNext;
		newItem.m_itPrev = itAllocedItem;
		newItem.m_isBegin = false;
		newItem.m_isEnd = (itAllocedItem->m_isEnd) ? true : false;
		m_itemList.emplace_back(newItem);

		itAllocedItem->m_itNext = std::prev(m_itemList.end());
		itAllocedItem->m_isEnd = false;
		if (!m_itemList.back().m_isEnd) {
			m_itemList.back().m_itNext->m_itPrev = std::prev(m_itemList.end());
		}
		
		return ret;
	}

	void 
		DescriptorChankAllocator::Free(DescriptorChankInfo const& _info) {
		std::lock_guard<std::mutex> lock(m_mutex);
		_info.itItem->m_isAllocated = false;
		auto mergedIt = _info.itItem;
		if (!_info.itItem->m_isBegin &&
			!_info.itItem->m_itPrev->m_isAllocated) {
			//前のfreeリストと結合
			auto prev = _info.itItem->m_itPrev;
			prev->m_itNext = _info.itItem->m_itNext;
			prev->m_isEnd = _info.itItem->m_isEnd;
			if (!prev->m_isEnd) {
				prev->m_itNext->m_itPrev = prev;
			}
			m_itemList.erase(_info.itItem);
			mergedIt = prev;
		}
		if (!mergedIt->m_isEnd &&
			!mergedIt->m_itNext->m_isAllocated) {
			//freeリスト結合
			auto next = mergedIt->m_itNext;
			mergedIt->m_itNext = next->m_itNext;
			mergedIt->m_isEnd = next->m_isEnd;
			if (!mergedIt->m_isEnd) {//次のアイテムがあるなら関係構築
				mergedIt->m_itNext->m_itPrev = mergedIt;
			}
			m_itemList.erase(next);
		}
	}

	uint32_t 
		DescriptorChankAllocator::GetSize(DescriptorChankAllocator::ItemList::iterator& _item) {
		if (_item->m_isEnd) {
			return m_hItemEnd - _item->m_ItemHandleForStart;
		}
		return _item->m_itNext->m_ItemHandleForStart - _item->m_ItemHandleForStart;
	}

	void 
		DescriptorChankInfo::Free() {
		if (pAllocator == nullptr) {
			assert(false);
			return;
		}
		pAllocator->Free(*this);
		pAllocator = nullptr;
		NumDescriptors = 0;
	}
}
