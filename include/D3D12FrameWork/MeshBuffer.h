#pragma once
#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/ComPtr.h"
#include "D3D12FrameWork/VertexBuffer.h"
#include "D3D12FrameWork/IndexBuffer.h"
#include <span>

namespace D3D12FrameWork {
	struct MaterialInfo
	{
		std::string MaterialName;
	};
	class D3DDevice;
	class CommandList;
	class MeshBuffer
	{
	public:
		MeshBuffer() = default;
		~MeshBuffer() = default;
		DECLMOVECOPY(MeshBuffer);

		bool Init(
			class ShaderInputDescs const&
		);

		template<typename T>
		bool SetVertices(
			D3DDevice* _pDev,
			CommandList* _pCmdList,
			std::span<T> const _vertices,
			uint32_t _slotNum
		) {
			if (m_vertexBuffs.size() <= _slotNum ||
				m_vertexBuffs[_slotNum].get() == nullptr) {
				assert(false);
				return false;
			}
			if (_slotNum == 0) {//perVerticesのデータ
				m_numVerts = static_cast<uint32_t>(_vertices.size());
			}
			else {//perInstanceのデータ
				m_numInstances = static_cast<uint32_t>(_vertices.size());
			}
			return m_vertexBuffs[_slotNum]->SetVertices<T>(
				_pDev, _pCmdList, _vertices
				);
		}

		bool SetIndices(
			D3DDevice* _pDev,
			CommandList* _pCmdList,
			std::span<uint32_t> const _indices
		) {
			return m_indexBuff->SetIndices(
				_pDev, _pCmdList, _indices
			);
		}

		auto* const GetVBView(uint32_t _slotNum)const {
			assert(m_vertexBuffs.size() > _slotNum);
			return m_vertexBuffs[_slotNum]->GetVBView();
		}
		auto* const GetIBView()const {
			return m_indexBuff->GetIBView();
		}
		auto SlotNum()const {
			return m_vertexBuffs.size();
		}
		auto VerticesNum()const {
			return m_numVerts;
		}
		auto InstancesNum()const {
			return m_numInstances;
		}

	private:
		std::vector<unqPtr<VertexBuffer>> m_vertexBuffs;
		uint32_t m_numVerts;
		uint32_t m_numInstances;
		unqPtr<IndexBuffer> m_indexBuff;
	};
}
