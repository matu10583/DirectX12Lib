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
	class IndexBuffer
	{
	public:
		IndexBuffer();
		~IndexBuffer();
		DECLMOVECOPY(IndexBuffer)


		bool Init(
			D3DDevice* _pDev,
			CommandList* _pCmdList,
			std::span<uint32_t> const _indices
		) {
			if (m_pBuffer.get() != nullptr) {
				m_pBuffer.reset();
			}
			m_pBuffer = std::make_unique<Buffer>();
			if (!m_pBuffer->InitAsDefaultBuffer(
				_pDev,
				static_cast<uint32_t>(sizeof(uint32_t) * _indices.size()),
				_pCmdList,
				reinterpret_cast<uint8_t const*>(_indices.data())
			)) {
				assert(false);
				return false;
			}

			//viewの作製
			m_ibv.BufferLocation = m_pBuffer->GetGPUVirtualAddress();
			m_ibv.SizeInBytes = static_cast<UINT>(sizeof(uint32_t) * _indices.size());
			m_ibv.Format = DXGI_FORMAT_R32_UINT;

			return true;
		}


		bool SetIndices(
			D3DDevice* _pDev,
			CommandList* _pCmdList,
			std::span<uint32_t> const _indices
		) {
			m_numIndices = _indices.size();
			//サイズが変わっていたら作り直す。
			if (m_ibv.SizeInBytes != sizeof(uint32_t) * _indices.size()) {

				return Init(
					_pDev, _pCmdList, _indices
				);
			}

			if (!m_pBuffer->CopyToResource(
				_pDev, _pCmdList,
				reinterpret_cast<uint8_t const*>(_indices.data()),
				sizeof(uint32_t) * _indices.size()
			)) {
				assert(false);
				return false;
			}

			m_ibv.BufferLocation = m_pBuffer->GetGPUVirtualAddress();
			m_ibv.SizeInBytes = static_cast<UINT>(sizeof(uint32_t) * _indices.size());
			m_ibv.Format = DXGI_FORMAT_R32_UINT;


			return true;
		}

		auto const* GetIBView()const {
			return &m_ibv;
		}

		uint32_t GetSize()const {
			return m_numIndices;
		}

	private:
		void Term();

		unqPtr<Buffer> m_pBuffer;
		D3D12_INDEX_BUFFER_VIEW m_ibv;
		uint32_t m_numIndices;

	};
}
