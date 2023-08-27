#pragma once
#include <string>
#include "D3D12FrameWork/MaterialSet.h"
#include "D3D12FrameWork/MeshSet.h"
#include <span>
namespace D3D12FrameWork {
	class RenderComponent
	{
	public:
		//ここでpsoに登録する.render component一つがpso一つに対応する．render componentはいわば特定のpsoに対するinputを意味する．
		RenderComponent(class IPipelineStateObject&);
		~RenderComponent();
		DECLMOVECOPY(RenderComponent);
		//descriptorとpsoからfactoryを通じてマテリアルセットを作製
		bool CreateMaterialSet(
			std::span<struct MaterialSetDescriptor> const&);
		//bool CreateMeshSet(struct MeshSetDescriptor const&) {}

		auto Material(uint32_t _index) const {
			return m_pMaterialSets[_index].get();
		}

		//descriptorとpsoからfactoryを通じてマテリアルセットを作製
		bool CreateMeshSet(std::string_view const _meshName);
		//bool CreateMeshSet(struct MeshSetDescriptor const&) {}

		auto Mesh() const {
			return m_pMeshSet.get();
		}

		void AfterDraw(uint32_t _index) {
			//描画後の後始末
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
