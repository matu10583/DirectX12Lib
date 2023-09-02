#include "D3D12FrameWork/RootParameterBuffer.h"

namespace D3D12FrameWork {
	bool RootParameterBuffer::Init(RootParamRegisterDesc const& rpRegDesc,
		RootSignature::RSRootParameterDesc const& rpDesc,
		D3DDevice* pDev,
		uint32_t _bufferCount) {
		for (auto i = 0u; i < rpRegDesc.RangeDescs.size();i++) {
			if (rpDesc.Ranges[i].Type == D3D12_DESCRIPTOR_RANGE_TYPE_CBV) {
				auto tmp = std::make_unique<ConstantBufferSet>();
				if (!tmp->InitFromRegisterDescWithRange(
					rpRegDesc.ConstantBindDesc, rpRegDesc.RangeDescs[i].Registers,
					rpDesc.Ranges[i].BaseShaderRegister,
					rpDesc.Ranges[i].NumDescriptors,
					rpDesc.BuffType,
					pDev,
					_bufferCount)) {
					assert(false);
					return false;
				}
				m_constantRangeBuffers.emplace_back(std::move(tmp));
				m_orderedRangeSet.emplace_back(m_constantRangeBuffers.back().get());
			}
			else if(rpDesc.Ranges[i].Type == D3D12_DESCRIPTOR_RANGE_TYPE_SRV) {
				auto tmp = std::make_unique<TextureSet>();
				if (!tmp->InitFromRegisterDescWithRange(
					rpRegDesc.TextureBindDesc, rpRegDesc.RangeDescs[i].Registers,
					rpDesc.Ranges[i].BaseShaderRegister,
					rpDesc.Ranges[i].NumDescriptors,
					pDev)) {
					assert(false);
					return false;
				}
				m_textureRangeBuffers.emplace_back(std::move(tmp));
				m_orderedRangeSet.emplace_back(m_textureRangeBuffers.back().get());
			}
			else if (rpDesc.Ranges[i].Type == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER) {
				auto tmp = std::make_unique<SamplerSet>();
				if (!tmp->InitFromRegisterDescWithRange(
					rpRegDesc.SamplerBindDesc, rpRegDesc.RangeDescs[i].Registers,
					rpDesc.Ranges[i].BaseShaderRegister,
					rpDesc.Ranges[i].NumDescriptors,
					pDev)) {
					assert(false);
					return false;
				}
				m_samplerRangeBuffers.emplace_back(std::move(tmp));
				m_orderedRangeSet.emplace_back(m_samplerRangeBuffers.back().get());
			}
		}
		return true;
	}


}
