#include "D3D12FrameWork/RootSignature.h"
#include "D3D12FrameWork/D3DDevice.h"

namespace D3D12FrameWork {
	bool RootSignature::Init(//後でrpDescから初期化するようにする．
		Set<ShaderBlob::ShaderType> const& _shader_flag,
		D3DDevice* _pDev
		) {
		//ALL(BTU,S),VS(BTU,S),PS(BTU,S)後から動的に指定できるようにする
		D3D12_ROOT_PARAMETER param[6] = {};
		m_rpDescs.resize(6);
		
		//SAmpler,CBVの2つ．どうせあとでUAVも作る
		D3D12_DESCRIPTOR_RANGE glbBTURange[2] = {};		
		m_rpDescs[0].Ranges.resize(2);
		
		glbBTURange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		glbBTURange[0].BaseShaderRegister = 0;
		glbBTURange[0].RegisterSpace = 0;
		glbBTURange[0].OffsetInDescriptorsFromTableStart =
			D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		//多めにレジスタをとっても使わなければ問題ない．多い分には問題ないってまいくろそふとが言ってた．
		glbBTURange[0].NumDescriptors = MAX_GLOABAL_REGISTER_NUM;
		m_rpDescs[0].Ranges[0].Type = glbBTURange[0].RangeType;
		m_rpDescs[0].Ranges[0].NumDescriptors = glbBTURange[0].NumDescriptors;
		m_rpDescs[0].Ranges[0].BaseShaderRegister = glbBTURange[0].BaseShaderRegister;



		glbBTURange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		glbBTURange[1].BaseShaderRegister = 0;
		glbBTURange[1].RegisterSpace = 0;
		glbBTURange[1].OffsetInDescriptorsFromTableStart =
			D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		glbBTURange[1].NumDescriptors = MAX_GLOABAL_REGISTER_NUM;
		m_rpDescs[0].Ranges[1].Type = glbBTURange[1].RangeType;
		m_rpDescs[0].Ranges[1].NumDescriptors = glbBTURange[1].NumDescriptors;
		m_rpDescs[0].Ranges[1].BaseShaderRegister = glbBTURange[1].BaseShaderRegister;

		param[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		param[0].DescriptorTable.NumDescriptorRanges = _countof(glbBTURange);
		param[0].DescriptorTable.pDescriptorRanges = glbBTURange;
		m_rpDescs[0].BuffType = BufferType::BUFFER_TYPE_UPLOAD;
		m_rpDescs[0].VisibleShader = ShaderBlob::NONE;

		
		D3D12_DESCRIPTOR_RANGE glbSRange[1] = {};
		m_rpDescs[1].Ranges.resize(1);

		glbSRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
		glbSRange[0].BaseShaderRegister = MAX_STATIC_SAMPLER_NUM;
		glbSRange[0].RegisterSpace = 0;
		glbSRange[0].OffsetInDescriptorsFromTableStart =
			D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		glbSRange[0].NumDescriptors = MAX_GLOABAL_REGISTER_NUM- MAX_STATIC_SAMPLER_NUM;
		m_rpDescs[1].Ranges[0].Type = glbSRange[0].RangeType;
		m_rpDescs[1].Ranges[0].NumDescriptors = glbSRange[0].NumDescriptors;
		m_rpDescs[1].Ranges[0].BaseShaderRegister = glbSRange[0].BaseShaderRegister;


		param[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		param[1].DescriptorTable.NumDescriptorRanges = _countof(glbSRange);
		param[1].DescriptorTable.pDescriptorRanges = glbSRange;
		m_rpDescs[1].BuffType = BufferType::BUFFER_TYPE_UPLOAD;
		m_rpDescs[1].VisibleShader = ShaderBlob::NONE;
		
		D3D12_DESCRIPTOR_RANGE vsBTURange[2] = {};
		m_rpDescs[2].Ranges.resize(2);

		vsBTURange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		vsBTURange[0].BaseShaderRegister = MAX_GLOABAL_REGISTER_NUM;
		vsBTURange[0].RegisterSpace = 0;
		vsBTURange[0].OffsetInDescriptorsFromTableStart =
			D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		vsBTURange[0].NumDescriptors = MAX_REGISTER_NUM - MAX_GLOABAL_REGISTER_NUM;
		m_rpDescs[2].Ranges[0].Type = vsBTURange[0].RangeType;
		m_rpDescs[2].Ranges[0].NumDescriptors = vsBTURange[0].NumDescriptors;
		m_rpDescs[2].Ranges[0].BaseShaderRegister = vsBTURange[0].BaseShaderRegister;


		vsBTURange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		vsBTURange[1].BaseShaderRegister = MAX_GLOABAL_REGISTER_NUM;
		vsBTURange[1].RegisterSpace = 0;
		vsBTURange[1].OffsetInDescriptorsFromTableStart =
			D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		vsBTURange[1].NumDescriptors = MAX_REGISTER_NUM - MAX_GLOABAL_REGISTER_NUM;
		m_rpDescs[2].Ranges[1].Type = vsBTURange[1].RangeType;
		m_rpDescs[2].Ranges[1].NumDescriptors = vsBTURange[1].NumDescriptors;
		m_rpDescs[2].Ranges[1].BaseShaderRegister = vsBTURange[1].BaseShaderRegister;

		param[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		param[2].DescriptorTable.NumDescriptorRanges = _countof(vsBTURange);
		param[2].DescriptorTable.pDescriptorRanges = vsBTURange;
		m_rpDescs[2].BuffType = BufferType::BUFFER_TYPE_UPLOAD;
		m_rpDescs[2].VisibleShader = ShaderBlob::VERTEX;

		D3D12_DESCRIPTOR_RANGE vsSRange[1] = {};
		m_rpDescs[3].Ranges.resize(1);

		vsSRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
		vsSRange[0].BaseShaderRegister = MAX_GLOABAL_REGISTER_NUM;
		vsSRange[0].RegisterSpace = 0;
		vsSRange[0].OffsetInDescriptorsFromTableStart =
			D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		vsSRange[0].NumDescriptors = MAX_REGISTER_NUM - MAX_GLOABAL_REGISTER_NUM;
		m_rpDescs[3].Ranges[0].Type = vsSRange[0].RangeType;
		m_rpDescs[3].Ranges[0].NumDescriptors = vsSRange[0].NumDescriptors;
		m_rpDescs[3].Ranges[0].BaseShaderRegister = vsSRange[0].BaseShaderRegister;


		param[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		param[3].DescriptorTable.NumDescriptorRanges = _countof(vsSRange);
		param[3].DescriptorTable.pDescriptorRanges = vsSRange;
		m_rpDescs[3].BuffType = BufferType::BUFFER_TYPE_UPLOAD;
		m_rpDescs[3].VisibleShader = ShaderBlob::VERTEX;

		D3D12_DESCRIPTOR_RANGE psBTURange[2] = {};
		m_rpDescs[4].Ranges.resize(2);

		psBTURange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		psBTURange[0].BaseShaderRegister = MAX_GLOABAL_REGISTER_NUM;
		psBTURange[0].RegisterSpace = 0;
		psBTURange[0].OffsetInDescriptorsFromTableStart =
			D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		psBTURange[0].NumDescriptors = MAX_REGISTER_NUM - MAX_GLOABAL_REGISTER_NUM;
		m_rpDescs[4].Ranges[0].Type = psBTURange[0].RangeType;
		m_rpDescs[4].Ranges[0].NumDescriptors = psBTURange[0].NumDescriptors;
		m_rpDescs[4].Ranges[0].BaseShaderRegister = psBTURange[0].BaseShaderRegister;


		psBTURange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		psBTURange[1].BaseShaderRegister = MAX_GLOABAL_REGISTER_NUM;
		psBTURange[1].RegisterSpace = 0;
		psBTURange[1].OffsetInDescriptorsFromTableStart =
			D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		psBTURange[1].NumDescriptors = MAX_REGISTER_NUM - MAX_GLOABAL_REGISTER_NUM;
		m_rpDescs[4].Ranges[1].Type = psBTURange[1].RangeType;
		m_rpDescs[4].Ranges[1].NumDescriptors = psBTURange[1].NumDescriptors;
		m_rpDescs[4].Ranges[1].BaseShaderRegister = psBTURange[1].BaseShaderRegister;

		param[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		param[4].DescriptorTable.NumDescriptorRanges = _countof(psBTURange);
		param[4].DescriptorTable.pDescriptorRanges = psBTURange;
		m_rpDescs[4].BuffType = BufferType::BUFFER_TYPE_UPLOAD;
		m_rpDescs[4].VisibleShader = ShaderBlob::PIXEL;

		D3D12_DESCRIPTOR_RANGE psSRange[1] = {};
		m_rpDescs[5].Ranges.resize(1);

		psSRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
		psSRange[0].BaseShaderRegister = MAX_GLOABAL_REGISTER_NUM;
		psSRange[0].RegisterSpace = 0;
		psSRange[0].OffsetInDescriptorsFromTableStart =
			D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		psSRange[0].NumDescriptors = MAX_REGISTER_NUM - MAX_GLOABAL_REGISTER_NUM;
		m_rpDescs[5].Ranges[0].Type = psSRange[0].RangeType;
		m_rpDescs[5].Ranges[0].NumDescriptors = psSRange[0].NumDescriptors;
		m_rpDescs[5].Ranges[0].BaseShaderRegister = psSRange[0].BaseShaderRegister;


		param[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		param[5].DescriptorTable.NumDescriptorRanges = _countof(psSRange);
		param[5].DescriptorTable.pDescriptorRanges = psSRange;
		m_rpDescs[5].BuffType = BufferType::BUFFER_TYPE_UPLOAD;
		m_rpDescs[5].VisibleShader = ShaderBlob::PIXEL;

		//rootsignature作製
		D3D12_ROOT_SIGNATURE_DESC root_desc = {};
		root_desc.NumParameters = _countof(param);
		root_desc.pParameters = param;
		root_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		if ((_shader_flag & ShaderBlob::VERTEX).IsEmpty()) {
			root_desc.Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
		}
		if ((_shader_flag & ShaderBlob::PIXEL).IsEmpty()) {
			root_desc.Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
		}
		if ((_shader_flag & ShaderBlob::GEOMETRY).IsEmpty()) {
			root_desc.Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
		}
		root_desc.NumStaticSamplers = static_cast<UINT>(m_staticSamplers.size());
		root_desc.pStaticSamplers = nullptr;
		std::vector<D3D12_STATIC_SAMPLER_DESC> smp_descs;
		smp_descs.resize(root_desc.NumStaticSamplers);
		for (int i = 0; auto const& d:m_staticSamplers) {
			smp_descs[i] = d.second;
		}
		if (smp_descs.size() != 0) {
			root_desc.pStaticSamplers = smp_descs.data();
		}

		ComPtr<ID3DBlob> rootSigBlob{};
		ComPtr<ID3DBlob> errorBlob{};
		auto hr = D3D12SerializeRootSignature(
			&root_desc,
			D3D_ROOT_SIGNATURE_VERSION_1_0,
			&rootSigBlob,
			&errorBlob
		);
		if (FAILED(hr)) {
			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			}
			assert(false);
			return false;
		}

		hr = _pDev->GetDev()->CreateRootSignature(
			0,
			rootSigBlob->GetBufferPointer(),
			rootSigBlob->GetBufferSize(),
			IID_PPV_ARGS(m_pRootSignature.ReleaseAndGetAddressOf())
		);
		RETURNIFFAILED(hr);

		return true;
	}

	bool 
		RootSignature::AddStaticSampler(
			std::string_view _sampler_name,
			UINT _registerNum,
			D3D12_SAMPLER_DESC const& smp_desc
		) {
		if (_registerNum >= MAX_STATIC_SAMPLER_NUM) assert(false);
		auto _smp_name = std::string(_sampler_name.begin(), _sampler_name.end());
		if (m_staticSamplers.count(_smp_name)) return false;
		m_staticSamplers[_smp_name] = D3D12_STATIC_SAMPLER_DESC{
			.Filter = smp_desc.Filter,
			.AddressU = smp_desc.AddressU,
			.AddressV = smp_desc.AddressV,
			.AddressW = smp_desc.AddressW,
			.MipLODBias = smp_desc.MipLODBias,
			.MaxAnisotropy = smp_desc.MaxAnisotropy,
			.ComparisonFunc = smp_desc.ComparisonFunc,
			.BorderColor=D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,//staticは数パターンしか決められないらしい.とりあえずこれで困りはしないと思う
			.MinLOD = smp_desc.MinLOD,
			.MaxLOD = smp_desc.MaxLOD,
			.ShaderRegister = _registerNum,
			.RegisterSpace = 0,
			.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL//pixel以外で使うことあるのか？
		};

		return true;
	}
}
