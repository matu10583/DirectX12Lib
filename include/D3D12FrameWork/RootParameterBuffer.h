#pragma once
#include "D3D12FrameWork/ComPtr.h"
#include "D3D12FrameWork/PipelineStateObject.h"
#include "D3D12FrameWork/RootSignature.h"
#include "D3D12FrameWork/ConstantBufferSet.h"
#include "D3D12FrameWork/TextureSet.h"
#include "D3D12FrameWork/SamplerSet.h"

namespace D3D12FrameWork {
	enum RPBufferType
	{
		CBV_SRV_UAV=0,
		RTV,
		DSV,
		SMP
	};

	class RootParameterBuffer
	{
	public:

		RootParameterBuffer() = default;
		virtual ~RootParameterBuffer() = default;
		DECLMOVECOPY(RootParameterBuffer);
		bool Init(RootParamRegisterDesc const&,
			RootSignature::RSRootParameterDesc const&,
			D3DDevice*,
			uint32_t _bufferCount);
		ConstantBufferSet* FindCBufferFromStructName(std::string_view _structName)const {
			for (auto const& cb : m_constantRangeBuffers) {
				if (cb->HasStruct(_structName)) {
					return cb.get();
				}
			}
			return nullptr;
		}
		ConstantBufferSet* FindCBufferFromVariableName(std::string_view _varName, 
			size_t* const _offset = nullptr)const {

			for (auto const& cb : m_constantRangeBuffers) {
				if (cb->HasVariable(_varName, _offset)) {
					return cb.get();
				}
			}
			return nullptr;
		}
		TextureSet* FindTextureFromRegisterName(std::string_view _regName)const {
			for (auto const& tex : m_textureRangeBuffers) {
				if (tex->HasKey(_regName)) {
					return tex.get();
				}
			}
			return nullptr;
		}
		SamplerSet* FindSamplerFromRegisterName(std::string_view _regName)const {
			for (auto const& smp : m_samplerRangeBuffers) {
				if (smp->HasKey(_regName)) {
					return smp.get();
				}
			}
			return nullptr;
		}

		std::vector<IDescriptorRangeSet*> const& GetRanges()const {
			return m_orderedRangeSet;
		}
		std::vector<ConstantBufferSet*> const GetConstants()const {
			std::vector<ConstantBufferSet*> ret;
			std::transform(m_constantRangeBuffers.begin(), m_constantRangeBuffers.end(),
				std::back_inserter(ret), [](auto const& ptr) {
					return ptr.get();
				});
			return ret;
		}
		std::vector<TextureSet*> const GetTextures()const {
			std::vector<TextureSet*> ret;
			std::transform(m_textureRangeBuffers.begin(), m_textureRangeBuffers.end(),
				std::back_inserter(ret), [](auto const& ptr) {
					return ptr.get();
				});
			return ret;
		}
		std::vector<SamplerSet*> const GetSamplers()const {
			std::vector<SamplerSet*> ret;
			std::transform(m_samplerRangeBuffers.begin(), m_samplerRangeBuffers.end(),
				std::back_inserter(ret), [](auto const& ptr) {
					return ptr.get();
				});
			return ret;
		}

		RPBufferType const GetType()const {
			if (
				!m_constantRangeBuffers.empty() ||
				!m_textureRangeBuffers.empty()
				) return RPBufferType::CBV_SRV_UAV;
			if (
				!m_samplerRangeBuffers.empty()
				) return RPBufferType::CBV_SRV_UAV;
		}

	private:

		//ƒŒƒ“ƒW‚Ì‡”Ô•t‚¯‚Í‚±‚ê‚Ås‚¤
		std::vector<IDescriptorRangeSet*> m_orderedRangeSet;
		std::vector<unqPtr<ConstantBufferSet>> m_constantRangeBuffers;
		std::vector<unqPtr<TextureSet>> m_textureRangeBuffers;
		std::vector<unqPtr<SamplerSet>> m_samplerRangeBuffers;


	};
}



