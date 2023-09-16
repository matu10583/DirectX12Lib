#pragma once
#include "D3D12FrameWork/MSPipelineStateObject.h"
#include "D3D12FrameWork/GraphicPipelineStateObject.h"
#include "D3D12FrameWork/RootSignature.h"
#include "D3D12FrameWork/ShaderBindInfo.h"
#include "D3D12FrameWork/D3DDevice.h"
#include <stdexcept>
#include <string>
#include <algorithm>


namespace D3D12FrameWork {
	template<ShaderBlob::ShaderType... ST>
	bool MSPipelineStateObject<ST...>::PrepareRootSignature(
		RootSignature* _pRS
	) {
		m_rootParamsRegDescs.Init(_pRS->GetRootParamDescs());
		m_defaultRpBuffNames.resize(_pRS->GetRootParamDescs().size());
		m_pRefRootSignature = _pRS;
		return true;
	}

	template<ShaderBlob::ShaderType... ST>
	bool MSPipelineStateObject<ST...>::EndPrepareAndInit(
		GraphicPipelineStateDesc const& _psoDesc,
		D3DDevice* _pDevice) {
		MeshShaderPipelineStateDesc psoDesc = {};
		//rs
		psoDesc.RootSignature = m_pRefRootSignature->GetDxRootSignature();
		//shader
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
		SETSHADER(AMPLIFY, AS);
		SETSHADER(MESH, MS);

#undef SETSHADER

		psoDesc.Rasterizer = _psoDesc.RasterizerState;
		psoDesc.Blend = _psoDesc.BlendState;
		psoDesc.DepthStencil = _psoDesc.DepthStencil;
		psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
		psoDesc.SampleDesc = _psoDesc.SampleDesc;


		D3D12_RT_FORMAT_ARRAY rtFormat = {};
		UINT numRT = 0;
		for (size_t i = 0; i < m_outputDesc.GetSize(); i++) {
			auto elem = m_outputDesc.GetElement(i);
			if (elem.SemanticName == "SV_TARGET") {
				auto idx = elem.SemanticIndex;
				//一旦r8g8b8a8unormにしておく．これ以外使う状況は当分ないかと
				rtFormat.RTFormats[idx] = DXGI_FORMAT_R8G8B8A8_UNORM;
				numRT++;
			}
		}
		rtFormat.NumRenderTargets = numRT;
		psoDesc.RTFormats = rtFormat;

		psoDesc.DSFormat = DXGI_FORMAT_UNKNOWN;
		psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		D3D12_PIPELINE_STATE_STREAM_DESC descStream = {};
		descStream.SizeInBytes = sizeof(psoDesc);
		descStream.pPipelineStateSubobjectStream = &psoDesc;

		auto hr = _pDevice->GetDev()->CreatePipelineState(
			&descStream,
			IID_PPV_ARGS(m_pPSO.ReleaseAndGetAddressOf())
		);
		RETURNIFFAILED(hr);
		return true;
	}


}

