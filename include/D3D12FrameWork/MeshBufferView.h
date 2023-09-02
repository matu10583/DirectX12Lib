#pragma once
#include "D3D12FrameWork/Common.h"
#include <d3d12.h>

namespace D3D12FrameWork {
	class MeshBuffer;
	class MeshBufferView
	{
	public:
		MeshBufferView() = delete;
		MeshBufferView(MeshBuffer& _mb)
			:m_meshBuff(_mb){}
		~MeshBufferView() = default;
		MeshBufferView(MeshBufferView&& old)noexcept
			:m_meshBuff(old.m_meshBuff){}
		MeshBufferView& operator= (MeshBufferView&& r) {
			m_meshBuff = r.m_meshBuff;
		}


		std::vector<D3D12_VERTEX_BUFFER_VIEW> const GetVBViews() const;
		D3D12_VERTEX_BUFFER_VIEW const* const GetVBView(uint32_t _slotNum) const;
		uint32_t NumVertices()const;
		uint32_t NumInstances()const;
		uint32_t NumIndices(uint32_t _index)const;
	private:
		std::reference_wrapper<MeshBuffer>  m_meshBuff;
	};
}


