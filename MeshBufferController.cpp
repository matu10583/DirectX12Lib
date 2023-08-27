#include "D3D12FrameWork/MeshBufferController.h"


namespace D3D12FrameWork{
	MeshBufferController::MeshBufferController(
		MeshBuffer& _mb,
		D3DDevice* const _pdev
	)
		:m_pRefDev(_pdev)
		, m_meshBuff(_mb) {
		if (!m_cmdList.Init(_pdev, D3D12_COMMAND_LIST_TYPE_DIRECT)) {
			assert(false);
		}
	}

}
