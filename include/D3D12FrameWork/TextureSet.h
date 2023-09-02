#pragma once
#include <unordered_map>
#include "D3D12FrameWork/Texture.h"
#include "D3D12FrameWork/TextureView.h"
#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/ShaderBindInfo.h"
#include "D3D12FrameWork/ConstantBufferSet.h"

namespace D3D12FrameWork {
	struct TextureViewSet
	{
		TextureViewSet() = default;
		~TextureViewSet() = default;
		DECLMOVECOPY(TextureViewSet);
		ShaderResourceView SRV;
		unqPtr<RenderTargetView> pRTV;
	};
	struct TextureAndView
	{
		unqPtr<Texture> pTexture;
		TextureViewSet ViewSet;
		TextureAndView()
			:pTexture(nullptr)
			,ViewSet(){}
	};
	//TblRangeÇ™ëŒâûÇµÇ‹Ç∑ÅD
	class TextureSet:public IDescriptorRangeSet
	{

	public:
		TextureSet()
			:m_textureMap()
			,m_textureViews()
			,m_doNeedUpdate(true)
		{}
		~TextureSet(){}
		DECLMOVECOPY(TextureSet);

		bool InitFromRegisterDescWithRange(
			std::unordered_map<std::string, ShaderRegisterDescs::TextureBindDesc>const&,
			std::vector<ShaderRegisterDescs::RegisterDesc> const&,
			UINT _rangeStart,
			UINT _rangeNum,
			class D3DDevice* _pDev);

		bool CreateResource(D3DDevice* _pdev, CommandList* _pcmdList,
			std::string_view _regName,
			DirectX::ScratchImage const* image,
			bool isRenderTarget=false);
		bool CreateResource(D3DDevice* _pdev, std::string_view _regName, 
			TextureDesc const& desc,
			bool isRenderTarget = false);

		RenderTargetView const* GetRenderTargetView(std::string_view _regName) {
			auto const regName = std::string(_regName.data());
			if (!m_textureMap.count(regName)) return nullptr;
			return m_textureMap[regName].ViewSet.pRTV.get();
		}

		bool HasKey(std::string_view _name) const{
			return m_textureMap.count(std::string(_name.data()));
		}

		std::vector<std::reference_wrapper<IResourceView const>> const& GetViews()const override {
			std::vector<std::reference_wrapper<IResourceView const>> ret;
			std::transform(m_textureViews.begin(), m_textureViews.end(),
				std::back_inserter(ret),
				[](TextureViewSet const* tvs) {
					return (tvs != nullptr) ? std::cref(tvs->SRV)
						: std::cref(ShaderResourceView::NullView());
				});
			return ret;
		}

		RangeCopyDesc GetCopyDesc()const override {
			if (!m_doNeedUpdate) {
				return RangeCopyDesc{
					.NumDescriptors = (UINT)m_textureViews.size()
				};
			}
			std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> handles;
			std::transform(m_textureViews.begin(), m_textureViews.end(),
				std::back_inserter(handles),
				[](TextureViewSet const* css) {
					return (css != nullptr) ? css->SRV.GetHeapInfo().m_CpuHandle
						: ShaderResourceView::NullView().GetHeapInfo().m_CpuHandle;
				});
			return RangeCopyDesc{
				.HandlesInRange = std::move(handles),
				.NumDescriptors = (UINT)m_textureViews.size(),
				//.Version=m_descriptorVersion
			};
		}
		size_t NumViews() const override { return m_textureViews.size(); }
		//void UpdateDescriptorHeap() {
		//	m_descriptorVersion = (m_descriptorVersion == UINT_MAX) ?
		//		0 : m_descriptorVersion + 1;
		//}
		//unsigned int const DescriptorHeapVersion()const override{ return m_descriptorVersion; }
	private:
		std::unordered_map<std::string, TextureAndView> m_textureMap;
		std::vector<TextureViewSet*> m_textureViews;
		bool m_doNeedUpdate;
		//unsigned int m_descriptorVersion = 0;
	};
}



