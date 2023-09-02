#include "D3D12FrameWork/BufferView.h"
#include "D3D12FrameWork/D3DDevice.h"
#include "D3D12FrameWork/Buffer.h"

namespace D3D12FrameWork{
	ConstantBufferView* ConstantBufferView::m_pNullView = nullptr;

	ConstantBufferView::ConstantBufferView()
		:m_descInfo(){}
	ConstantBufferView::~ConstantBufferView() {
		Term();
	}
	bool 
		ConstantBufferView::Init(
			D3DDevice const* _pDev,
			Buffer const* _pBuff,
			const size_t _offset,
			const size_t _size
		) {
		if (_pDev == nullptr || _pBuff == nullptr) return false;
		auto const buffSize = (_size == 0) ? _pBuff->GetSize() : _size;
		auto const alignedSize= (buffSize + 0xff) & ~0xff;

		m_descInfo = _pDev->GetCbvSrvUavHeap()->Allocate();

		D3D12_CONSTANT_BUFFER_VIEW_DESC viewDesc = {};
		viewDesc.BufferLocation = _pBuff->GetGPUVirtualAddress() + _offset;
		viewDesc.SizeInBytes = alignedSize;

		_pDev->GetDev()->CreateConstantBufferView(
			&viewDesc,
			m_descInfo.m_CpuHandle
		);

		return true;
	}
}
