#pragma once
#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/CommandList.h"
#include "D3D12FrameWork/MeshBuffer.h"
#include "D3D12FrameWork/ModelLoader.h"
#include <span>

namespace D3D12FrameWork {
	class MeshBuffer;
	class D3DDevice;
	class MeshBufferController
	{
	public:
		MeshBufferController() = delete;
		MeshBufferController(MeshBuffer&,
			D3DDevice* const
		);
		~MeshBufferController() = default;
		MeshBufferController(MeshBufferController&& old)noexcept
			:m_meshBuff(old.m_meshBuff)
			, m_pRefDev(old.m_pRefDev)
			,m_cmdList(std::move(old.m_cmdList)){}
		MeshBufferController& operator= (MeshBufferController&& r) {
			m_meshBuff = r.m_meshBuff;
			m_pRefDev = r.m_pRefDev;
			m_cmdList = std::move(r.m_cmdList);
		}

		template<typename T>
		bool SetVertices(
			std::span<T>const _vertices,
			uint32_t _slotNum) {
			m_cmdList.Begin();
			if (!m_meshBuff.get().SetVertices(
				m_pRefDev, &m_cmdList, _vertices, _slotNum
			)) {
				assert(false);
				return false;
			}
			CommandList* pCmdList[] = { &m_cmdList };
			m_pRefDev->EndAndExecuteCommandList(pCmdList, 1);
		}

		//template<typename T>
		//bool SetVertices(
		//	std::filesystem::path const& _path,
		//	uint32_t _slotNum
		//) {
		//	auto vertices = std::span<T>();
		//	auto indices = std::span<unsigned short>();
		//	ModelLoader<T>::Load(
		//		_path, vertices, indices
		//	);
		//	
		//	return SetVertices<T>(vertices, _slotNum);
		//}

	private:
		std::reference_wrapper<MeshBuffer> m_meshBuff;
		D3DDevice* m_pRefDev;
		CommandList m_cmdList;//thread safe‚É‚·‚é
	};
}


