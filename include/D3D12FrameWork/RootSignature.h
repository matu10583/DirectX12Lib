#pragma once
#include <d3d12.h>
#include "D3D12FrameWork/ComPtr.h"
#include "D3D12FrameWork/ShaderSet.h"
#include "D3D12FrameWork/Set.h"
#include <unordered_map>
#include "D3D12FrameWork/Buffer.h"

namespace D3D12FrameWork {
	class RootSignature
	{
	public:
		struct RSDescriptorRangeDesc
		{
			D3D12_DESCRIPTOR_RANGE_TYPE Type;
			UINT BaseShaderRegister;
			UINT NumDescriptors;
		};
		struct RSRootParameterDesc
		{
			std::vector<RSDescriptorRangeDesc> Ranges;
			BufferType BuffType;
			ShaderBlob::ShaderType VisibleShader;
		};
		RootSignature()
			:m_pRootSignature()
			,m_staticSamplers(){
			if (MAX_GLOABAL_REGISTER_NUM - MAX_STATIC_SAMPLER_NUM > 1) {
				assert(false&&"GlobalÇ»ÉTÉìÉvÉâÅ[ÇÕ1à»è„Ç»Ç¢Ç∆Ç¢ÇØÇ»Ç¢");
			}
		}
		bool Init( 
			Set<ShaderBlob::ShaderType> const& _shader_flag,
			class D3DDevice* _pDev);
		bool AddStaticSampler(
			std::string_view,
			UINT _registerNum,
			D3D12_SAMPLER_DESC const&
		);
		GETTERPTR(ID3D12RootSignature, DxRootSignature, m_pRootSignature.Get());
		GETTERCONST(std::vector<RSRootParameterDesc>, RootParamDescs, m_rpDescs);
		const UINT MAX_REGISTER_NUM = 21;//0~20
		const UINT MAX_GLOABAL_REGISTER_NUM = 6;//0~5
		const UINT MAX_STATIC_SAMPLER_NUM = 5;//0~4
	private:
		ComPtr<ID3D12RootSignature> m_pRootSignature;
		std::unordered_map<std::string, D3D12_STATIC_SAMPLER_DESC>
			m_staticSamplers;
		std::vector<RSRootParameterDesc> m_rpDescs;
	};
}
