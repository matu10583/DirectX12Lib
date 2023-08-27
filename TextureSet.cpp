#include "D3D12FrameWork/TextureSet.h"

namespace D3D12FrameWork{
	bool 
		TextureSet::InitFromRegisterDescWithRange(
			std::unordered_map<std::string, ShaderRegisterDescs::TextureBindDesc>const& _texMap,
			std::vector<ShaderRegisterDescs::RegisterDesc> const& _regDesc,
			UINT _rangeStart,
			UINT _rangeNum,
			class D3DDevice* _pDev) {
		if (_pDev == nullptr) {
			return false;
		}

		//レジスタの順番と名前&arrayIdxの対応を覚えておく
		std::vector<std::pair<std::string_view, size_t>> regNames; regNames.resize(_rangeNum);
		for (auto const& tReg : _regDesc) {
			for (int i = 0; i < tReg.BindCount; i++) {
				regNames[tReg.BindPoint - _rangeStart + i] = std::make_pair(std::string_view(tReg.Name), i);
			}
		}
		m_textureViews.resize(_rangeNum);
		for (auto i = 0; i < _rangeNum; i++) {
			if (regNames[i].first.empty()) {
				//データが存在しない
				m_textureViews[i] = nullptr;
				continue;
			}
			//textureとそのビューを作る
			auto texName = std::string(regNames[i].first.data());
			m_textureMap[texName].pTexture.reset(
				new Texture()
			);
			if (!m_textureMap[texName].ViewSet.SRV.Init(
				_pDev
			)) {
				assert(false);
				return false;
			}
			m_textureViews[i] = &m_textureMap[texName].ViewSet;
		}
		//UpdateDescriptorHeap();
		return true;
	}

	bool 
		TextureSet::CreateResource(D3DDevice* _pdev, CommandList* _pcmdList,
			std::string_view _regName, DirectX::ScratchImage const* image,
			bool isRenderTarget) {
		auto const regName = std::string(_regName.data());
		if (!m_textureMap.count(regName)) return false;
		m_textureMap[regName].pTexture.reset(new Texture());
		if (!m_textureMap[regName].pTexture->InitFromDXImage(_pdev, _pcmdList, image)) {
			assert(false);
			return false;
		}
		//rtとして使わないならこれでok
		if (!m_textureMap[regName].ViewSet.SRV.UpdateView(_pdev, m_textureMap[regName].pTexture.get())) {
			assert(false);
			return false;
		}
		//UpdateDescriptorHeap();
		if (isRenderTarget == false) return true;
		//なければつくりませう
		if (m_textureMap[regName].ViewSet.pRTV == nullptr) {
			m_textureMap[regName].ViewSet.pRTV.reset(new RenderTargetView());
			m_textureMap[regName].ViewSet.pRTV->Init(_pdev);
		}

		if (!m_textureMap[regName].ViewSet.pRTV->UpdateView(
			_pdev, m_textureMap[regName].pTexture.get())) {
			assert(false);
			return false;
		}

		return true;
	}
	bool 
		TextureSet::CreateResource(D3DDevice* _pdev, std::string_view _regName,
			TextureDesc const& desc,
			bool isRenderTarget) {
		auto const regName = std::string(_regName.data());
		if (!m_textureMap.count(regName)) return false;
		m_textureMap[regName].pTexture.reset(new Texture());
		if (!m_textureMap[regName].pTexture->Init(_pdev, desc)) {
			assert(false);
			return false;
		}
		if (!m_textureMap[regName].ViewSet.SRV.UpdateView(_pdev, m_textureMap[regName].pTexture.get())) {
			assert(false);
			return false;
		}
		//UpdateDescriptorHeap();
		//rtとして使わないならこれでok
		if (isRenderTarget == false) return false;
		//なければつくりませう
		if (m_textureMap[regName].ViewSet.pRTV == nullptr) {
			m_textureMap[regName].ViewSet.pRTV.reset(new RenderTargetView());
			m_textureMap[regName].ViewSet.pRTV->Init(_pdev);
		}

		if (!m_textureMap[regName].ViewSet.pRTV->UpdateView(
			_pdev, m_textureMap[regName].pTexture.get())) {
			assert(false);
			return false;
		}

		return true;
	}
}
