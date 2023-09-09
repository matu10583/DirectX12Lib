#include "D3D12FrameWork/RenderComponent.h"
#include "D3D12FrameWork/GraphicPipelineStateObject.h"
#include "D3D12FrameWork/MaterialFactory.h"
#include "D3D12FrameWork/MeshFactory.h"

namespace D3D12FrameWork{
	RenderComponent::RenderComponent(IPipelineStateObject& _pso)
		:m_pso(_pso){
		_pso.AddRenderComponent(*this);
	}
	RenderComponent::~RenderComponent() {
		m_pso.get().RemoveRenderComponent(this);
	}

	bool
		RenderComponent::CreateMaterialSet(
			std::span<MaterialSetDescriptor> const& _msDesc
		) {
		m_pMaterialSets.resize(_msDesc.size());
		for (auto i = 0u; i < _msDesc.size();i++) {
			m_pMaterialSets[i] = MaterialFactory::Instance()->CreateMaterialSet(
				_msDesc[i], &(m_pso.get()));
		}

		return true;
	}

	bool
		RenderComponent::CreateMeshSet(
			std::string_view const _meshName
		) {
		m_pMeshSet = MeshFactory::Instance()->CreateMeshSet(
			_meshName, &(m_pso.get()));
		return true;
	}
}
