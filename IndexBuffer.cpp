#include "D3D12FrameWork/IndexBuffer.h"


namespace D3D12FrameWork {

	IndexBuffer::IndexBuffer()
		:m_pBuffer(nullptr)
		, m_ibv({}) {}

	IndexBuffer::~IndexBuffer() {
		Term();
	}


	void IndexBuffer::Term() {

	}
}
