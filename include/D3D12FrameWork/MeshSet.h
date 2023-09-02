#pragma once
#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/MeshBufferController.h"
#include "D3D12FrameWork/MeshBufferView.h"

namespace D3D12FrameWork {
	class MeshSet
	{
		friend class MeshFactory;
	public:
		MeshSet() = delete;
		MeshSet(class MeshBuffer* _pBuff, class D3DDevice* _pDev)
			:m_meshController(*_pBuff, _pDev)
			,m_meshView(*_pBuff){}
		~MeshSet() = default;
		DECLMOVECOPY(MeshSet);

		MeshBufferController& GetController() {
			return m_meshController;
		}

		MeshBufferView const& GetView() const {
			return m_meshView;
		}

	private:
		MeshBufferController m_meshController;
		MeshBufferView m_meshView;
	};
}

