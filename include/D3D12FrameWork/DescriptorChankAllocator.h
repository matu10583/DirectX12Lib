#pragma once
#include "D3D12FrameWork/DescriptorAllocator.h"

namespace D3D12FrameWork {
	struct DescriptorChankInfo
	{	

		struct Item
		{
			std::list<Item>::iterator m_itPrev;
			std::list<Item>::iterator m_itNext;
			uint32_t m_ItemHandleForStart;
			bool m_isAllocated;
			bool m_isBegin;
			bool m_isEnd;
		};
		class DescriptorChankAllocator* pAllocator;
		D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle;
		std::list<Item>::iterator itItem;
		size_t NumDescriptors;
		void Free();
		DescriptorChankInfo()
			:pAllocator(nullptr)
			,CpuHandle()
			,GpuHandle()
			,itItem()
			,NumDescriptors(0){}
	};
	class DescriptorChankAllocator
	{

	public:

		using ItemList = std::list<DescriptorChankInfo::Item>;
		DescriptorChankAllocator();
		~DescriptorChankAllocator();
		DECLMOVECOPY(DescriptorChankAllocator);

		bool Init(D3DDevice* _pDev, D3D12_DESCRIPTOR_HEAP_DESC const& _heapDesc);
		DescriptorChankInfo const Allocate(uint32_t _numDesc);
		void Free(DescriptorChankInfo const&);
		auto Type()const { return m_type; }
		auto Size()const { return m_numDesc; }
		auto IncrementSize() const { return m_IncrementSize; }
		auto GetHeap()const { return m_pHeap.Get(); }
		
	private:
		uint32_t GetSize(DescriptorChankAllocator::ItemList::iterator&);
		std::mutex m_mutex;

		D3D12_CPU_DESCRIPTOR_HANDLE m_hCpuHead;
		D3D12_GPU_DESCRIPTOR_HANDLE m_hGpuHead;

		ComPtr<ID3D12DescriptorHeap> m_pHeap;
		
		std::list<DescriptorChankInfo::Item> m_itemList;
		uint32_t m_IncrementSize;
		uint32_t m_hItemEnd;
		uint32_t m_numDesc;
		D3D12_DESCRIPTOR_HEAP_TYPE m_type;
		bool m_isGPUVisible;
	};	


}

