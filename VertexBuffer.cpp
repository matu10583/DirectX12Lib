#include "D3D12FrameWork/VertexBuffer.h"


namespace D3D12FrameWork{

	VertexBuffer::VertexBuffer()
		:m_pBuffer(nullptr)
		, m_vbv({}) {}

	VertexBuffer::~VertexBuffer() {
		Term();
	}






	void VertexBuffer::Term() {

	}
}
