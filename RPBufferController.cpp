#include "D3D12FrameWork/RPBufferController.h"
#include "D3D12FrameWork/RootParameterBuffer.h"
#include "D3D12FrameWork/ImageLoader.h"

namespace D3D12FrameWork{
	RPBufferController::RPBufferController(RootParameterBuffer& _rpBuff,
		D3DDevice* const _pDev)
		:m_rpBuff(_rpBuff)
		, m_pRefDev(_pDev) {
		if (!m_cmdList.Init(_pDev, D3D12_COMMAND_LIST_TYPE_DIRECT)) {
			assert(false);
		}
	}

	bool 
		RPBufferController::SetTexture(
			//お名前とファイル名．ローダーを別クラスで作っておく
			std::string_view regName,
			std::filesystem::path const& _fileName,
			uint32_t const mipL
		) {
		//フライウェイトにした
		auto dxImg = ImageLoader::Load2DxImg(_fileName, mipL);
		auto tex = m_rpBuff.get().FindTextureFromRegisterName(regName);
		if (tex == nullptr) {
			assert(false);
			return false;
		}
		m_cmdList.Begin();
		if (!tex->CreateResource(
			m_pRefDev, &m_cmdList, regName, dxImg
		)) {
			return false;
		}
		CommandList* pCmdList[] = {&m_cmdList};
		m_pRefDev->EndAndExecuteCommandList(pCmdList, 1);
		return true;
	}


	bool 
		RPBufferController::SetTexture(
			std::string_view regName,
			TextureDesc const& _texDesc
		) {
		auto tex = m_rpBuff.get().FindTextureFromRegisterName(regName);
		if (tex == nullptr) {
			assert(false);
			return false;
		}
		if (!tex->CreateResource(
			m_pRefDev, regName, _texDesc, true
		)) {
			return false;
		}
		return true;
	}


	bool
		RPBufferController::SetSampler(
			std::string_view regName,
			D3D12_SAMPLER_DESC const& _smpDesc
		) {
		auto smp = m_rpBuff.get().FindSamplerFromRegisterName(regName);
		if (smp == nullptr) {
			assert(false);
			return false;
		}
		if (!smp->CreateSampler(
			m_pRefDev, regName, _smpDesc
		)) {
			return false;
		}
		return true;
	}

	void 
		RPBufferController::AfterDraw() {
		auto const cbs = m_rpBuff.get().GetConstants();
		for (auto& cb : cbs) {
			cb->AfterDraw();
		}
	}
}
