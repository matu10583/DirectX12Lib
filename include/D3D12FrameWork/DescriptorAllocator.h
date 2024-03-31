#pragma once
#include <cstdint>
#include <d3d12.h>
#include <mutex>
#include "D3D12FrameWork/ComPtr.h"
#include "D3D12FrameWork/Common.h"

namespace D3D12FrameWork {
	class DescriptorAllocator;
	//ÉqÅ[ÉvÇÃèÓïÒ
	struct DescriptorInfo
	{	
		//ÉÅÉÇÉäÇÃä«óùèÓïÒ
		struct Item
		{
			uint32_t m_Index;
			Item* m_Next;
			//Item* m_Prev;
		};
		DescriptorInfo() = default;
		~DescriptorInfo() {
		}
		DECLMOVECOPY(DescriptorInfo);
		DescriptorAllocator* m_pAllocator;
		Item* m_pItem;
		D3D12_CPU_DESCRIPTOR_HANDLE m_CpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_GpuHandle;
		void Free();
	};

	class D3DDevice;

	class DescriptorAllocator
	{

	public:
		DescriptorAllocator();
		~DescriptorAllocator() {
			Term();
		}
		DECLMOVECOPY(DescriptorAllocator);

		bool Init(D3DDevice* dev, D3D12_DESCRIPTOR_HEAP_DESC const& desc );
		void Term();

		DescriptorInfo const& Allocate();
		void Free(DescriptorInfo const&);

	private:
		DescriptorInfo::Item* GetItem(uint32_t index);

		std::mutex m_mutex;

		unqPtr<uint8_t[]> m_pBuffer;
		D3D12_CPU_DESCRIPTOR_HANDLE m_hCpuHead;
		D3D12_GPU_DESCRIPTOR_HANDLE m_hGpuHead;

		DescriptorInfo::Item* m_pFreeList;
		uint32_t m_IncrementSize;
		D3D12_DESCRIPTOR_HEAP_DESC m_HeapDesc;
		ComPtr<ID3D12DescriptorHeap> m_pHeap;
		bool m_isGPUVisible;
	};

}

