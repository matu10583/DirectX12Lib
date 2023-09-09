
#include "D3D12FrameWork/GraphicPipelineStateObject.h"
#include "D3D12FrameWork/RootSignature.h"
#include "D3D12FrameWork/ShaderBindInfo.h"
#include "D3D12FrameWork/D3DDevice.h"
#include <stdexcept>
#include <string>
#include <algorithm>

namespace D3D12FrameWork {
	template<ShaderBlob::ShaderType... ST>
	bool GraphicPipelineStateObject<ST...>::PrepareRootSignature(
		RootSignature* _pRS
	) {
		m_rootParamsRegDescs.Resize(_pRS->GetRootParamDescs().size());
		m_defaultRpBuffNames.resize(_pRS->GetRootParamDescs().size());
		m_pRefRootSignature = _pRS;
		return true;
	}

	template<ShaderBlob::ShaderType... ST>
	bool GraphicPipelineStateObject<ST...>::EndPrepareAndInit(
		GraphicPipelineStateDesc const& _psoDesc,
		D3DDevice* _pDevice) {

		std::vector<D3D12_INPUT_ELEMENT_DESC> in_elem{};
		in_elem.resize(m_inputDesc.GetSize());
		for (size_t i = 0; i < m_inputDesc.GetSize(); i++) {
			auto& in_desc = m_inputDesc.GetElement(i);
			in_elem[i].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
			in_elem[i].Format = in_desc.Format;
			in_elem[i].SemanticIndex = in_desc.SemanticIndex;
			in_elem[i].SemanticName = in_desc.SemanticName.c_str();
			in_elem[i].InputSlot = in_desc.Register;
			in_elem[i].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			in_elem[i].InstanceDataStepRate = 0;
			if (in_elem[i].InputSlot >= 1) {//slotが0でないものはインスタンス用
				in_elem[i].InputSlotClass =
					D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
				in_elem[i].InstanceDataStepRate = 1;
			}

		}//end for


		D3D12_INPUT_LAYOUT_DESC in_layout = {};
		in_layout.NumElements = in_elem.size();
		in_layout.pInputElementDescs = in_elem.data();

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		auto const& shaderFlag = m_shaderSet.GetShaderFlag();

		//shader定義用 
#define SETSHADER(SType,DescName)\
if constexpr(ShaderBlob::FindShaderType<ShaderBlob::SType,ST...>::value){\
		if (!(shaderFlag & ShaderBlob::SType).IsEmpty()) {\
		D3D12_SHADER_BYTECODE shader = {};\
		auto const& shaderBlob = m_shaderSet.GetShader<ShaderBlob::SType>().GetBlob();\
		shader.pShaderBytecode = shaderBlob->GetBufferPointer();\
		shader.BytecodeLength = shaderBlob->GetBufferSize();\
		psoDesc.DescName = shader;\
		}\
};
		SETSHADER(PIXEL, PS);
		SETSHADER(VERTEX, VS);

#undef SETSHADER


		psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
		psoDesc.RasterizerState = _psoDesc.RasterizerState;
		psoDesc.BlendState = _psoDesc.BlendState;
		psoDesc.InputLayout = in_layout;
		psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
		psoDesc.PrimitiveTopologyType = _psoDesc.PrimitiveTopologyType;
		psoDesc.SampleDesc = _psoDesc.SampleDesc;
		psoDesc.DepthStencilState = _psoDesc.DepthStencil;

		UINT numRT = 0;
		for (size_t i = 0; i < m_outputDesc.GetSize(); i++) {
			auto elem = m_outputDesc.GetElement(i);
			if (elem.SemanticName == "SV_TARGET") {
				auto idx = elem.SemanticIndex;
				//一旦r8g8b8a8unormにしておく．これ以外使う状況は当分ないかと
				psoDesc.RTVFormats[idx] = DXGI_FORMAT_R8G8B8A8_UNORM;
				numRT++;
			}
		}
		psoDesc.NumRenderTargets = numRT;
		psoDesc.pRootSignature = m_pRefRootSignature->GetDxRootSignature();

		auto hr = _pDevice->GetDev()->CreateGraphicsPipelineState(
			&psoDesc,
			IID_PPV_ARGS(m_pPSO.ReleaseAndGetAddressOf())
		);
		RETURNIFFAILED(hr);
		return true;
	}
}

