#include "D3D12FrameWork/MeshFactory.h"
#include "D3D12FrameWork/GraphicPipelineStateObject.h"



namespace D3D12FrameWork{

	MeshFactory* MeshFactory::m_instance = nullptr;


	MeshFactory*
		MeshFactory::CreateMesh(
			std::string_view _meshName,
			IPipelineStateObject* _pso,
			bool useIndex
		) {
		auto const& inputDesc = _pso->GetInputDesc();
		auto tmpMB = std::make_unique<MeshBuffer>();
		if (!tmpMB->Init(inputDesc, useIndex)) {
			assert(false);
			return nullptr;
		}

		auto const meshName = std::string(_meshName.data());
		auto& mbMap = m_psoMeshBuffMap[_pso->GetName().data()].RPBuffs;
		if (mbMap.count(meshName)) {
			assert(false);
			return nullptr;
		}
		mbMap[meshName].reset(tmpMB.release());


		return this;
	}

	MeshBufferController
		MeshFactory::MeshController(
			std::string_view _matName,
			class IPipelineStateObject const* _pso) {
		auto psoName = _pso->GetName().data();
		if(!m_psoMeshBuffMap.count(psoName)||
			!m_psoMeshBuffMap[psoName].RPBuffs.count(_matName.data())) {
			assert(false);
		}
		return MeshBufferController(*m_psoMeshBuffMap[psoName].RPBuffs[_matName.data()].get(),
			m_pRefDev);
	}

	unqPtr<class MeshSet> 
		MeshFactory::CreateMeshSet(
			std::string_view const _meshName,
			IPipelineStateObject const* _pso
		) {
		auto psoName = std::string(_pso->GetName().data());
		if (!m_psoMeshBuffMap.count(psoName)) {
			throw std::runtime_error("指定されたpsoで使用されるMesh一つも登録されていません．");
		}
		auto& mbMap = m_psoMeshBuffMap[psoName].RPBuffs;
		if (!mbMap.count(_meshName.data())) {
			throw std::runtime_error("未作成のMesh");
		}
		auto* meshBuff = mbMap[_meshName.data()].get();
		auto ret = std::make_unique<MeshSet>(meshBuff, m_pRefDev);

		return ret;
	}

}
