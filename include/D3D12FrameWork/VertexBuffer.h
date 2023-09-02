#pragma once
#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/ComPtr.h"
#include "D3D12FrameWork/Vector.h"
#include <vector>
#include "D3D12FrameWork/ComPtr.h"
#include "D3D12FrameWork/Buffer.h"
#include"D3D12FrameWork/D3DDevice.h"
#include <span>

namespace D3D12FrameWork {

	class Buffer;
	class D3DDevice;
	class CommandList;
	//頂点データはcpu write once, gpu read several timeだと思う
	class VertexBuffer
	{
	public:
		VertexBuffer();
		~VertexBuffer();
		DECLMOVECOPY(VertexBuffer)

		template<typename T>
		bool Init(
			D3DDevice* _pDev,
			CommandList* _pCmdList,
			std::span<T> const _vertices
		) {
			if (m_pBuffer.get() != nullptr) {
				m_pBuffer.reset();
			}
			m_pBuffer = std::make_unique<Buffer>();
			if (!m_pBuffer->InitAsDefaultBuffer(
				_pDev,
				sizeof(T)*_vertices.size(),
				_pCmdList,
				reinterpret_cast<uint8_t const*>(_vertices.data())
			)) {
				assert(false);
				return false;
			}

			//viewの作製
			m_vbv.BufferLocation = m_pBuffer->GetGPUVirtualAddress();
			m_vbv.SizeInBytes = sizeof(T)*_vertices.size();
			m_vbv.StrideInBytes = sizeof(T);

			return true;
		}

		template<typename T>
		bool SetVertices(
			D3DDevice* _pDev,
			CommandList* _pCmdList,
			std::span<T> const _vertices
		) {
			//サイズが変わっていたら作り直す。
			if (m_vbv.SizeInBytes != sizeof(T) * _vertices.size() ||
				m_vbv.StrideInBytes != sizeof(T)) {

				return Init<T>(
					_pDev, _pCmdList, _vertices
					);
			}

			if (!m_pBuffer->CopyToResource(
				_pDev, _pCmdList,
				reinterpret_cast<uint8_t const*>(_vertices.data()),
				sizeof(T)*_vertices.size()
			)) {
				assert(false);
				return false;
			}

			m_vbv.BufferLocation = m_pBuffer->GetGPUVirtualAddress();
			m_vbv.SizeInBytes = sizeof(T) * _vertices.size();
			m_vbv.StrideInBytes = sizeof(T);


			return true;
		}

		auto const* GetVBView()const {
			return &m_vbv;
		}
		
	private:
		void Term();

		unqPtr<Buffer> m_pBuffer;
		D3D12_VERTEX_BUFFER_VIEW m_vbv;
	};
}


