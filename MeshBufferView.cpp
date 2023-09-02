#include "D3D12FrameWork/MeshBufferView.h"
#include "D3D12FrameWork/MeshBuffer.h"

namespace D3D12FrameWork{
	std::vector<D3D12_VERTEX_BUFFER_VIEW> const
		MeshBufferView::GetVBViews() const {
		auto size = m_meshBuff.get().SlotNum();
		auto ret = std::vector<D3D12_VERTEX_BUFFER_VIEW>();
		ret.resize(size);
		for (auto i = 0; i < size;i++) {
			ret[i] = *(m_meshBuff.get().GetVBView(i));
		}
		return ret;
	}
	D3D12_VERTEX_BUFFER_VIEW const* const
		MeshBufferView::GetVBView(uint32_t _slotNum)const {
		return m_meshBuff.get().GetVBView(_slotNum);
	}

	uint32_t 
		MeshBufferView::NumVertices()const {
		return m_meshBuff.get().VerticesNum();
	}
	uint32_t 
		MeshBufferView::NumInstances()const {
		return m_meshBuff.get().InstancesNum();
	}
	uint32_t
		MeshBufferView::NumIndices(uint32_t _index)const {
		return m_meshBuff.get().IndicesNum(_index);
	}
}
