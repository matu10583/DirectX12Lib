#pragma once
#include <string>
#include "D3D12FrameWork/MaterialSet.h"
#include "D3D12FrameWork/MeshSet.h"
#include <span>
namespace D3D12FrameWork {
	class RenderComponent
	{
	public:
		//������pso�ɓo�^����.render component���pso��ɑΉ�����Drender component�͂���Γ����pso�ɑ΂���input���Ӗ�����D
		RenderComponent(class IPipelineStateObject&);
		~RenderComponent();
		DECLMOVECOPY(RenderComponent);
		//descriptor��pso����factory��ʂ��ă}�e���A���Z�b�g���쐻
		bool CreateMaterialSet(
			std::span<struct MaterialSetDescriptor> const&);
		//bool CreateMeshSet(struct MeshSetDescriptor const&) {}

		auto Material(uint32_t _index) const {
			return m_pMaterialSets[_index].get();
		}

		//descriptor��pso����factory��ʂ��ă}�e���A���Z�b�g���쐻
		bool CreateMeshSet(std::string_view const _meshName);
		//bool CreateMeshSet(struct MeshSetDescriptor const&) {}

		auto Mesh() const {
			return m_pMeshSet.get();
		}

		void AfterDraw(uint32_t _index) {
			//�`���̌�n��
			auto matSize = m_pMaterialSets[_index]->Size();
			for (auto i = 0u; i < matSize; i++) {
				m_pMaterialSets[_index]->GetController(i).AfterDraw();
			}
		}
	private:
		std::vector<unqPtr<MaterialSet>> m_pMaterialSets;
		unqPtr<MeshSet> m_pMeshSet;
		std::reference_wrapper <IPipelineStateObject> m_pso;

	};

}
