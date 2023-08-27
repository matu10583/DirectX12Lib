#include "D3D12FrameWork/ShaderBindInfo.h"
#include "D3D12FrameWork/EnumNameConverter.h"
#include <iostream>
#include <sstream>

namespace {
	DXGI_FORMAT MaskToFormat(D3D_REGISTER_COMPONENT_TYPE _type,
		BYTE _mask) {
		std::stringstream ss;
		ss << "DXGI_FORMAT_";
		if (_mask & (1 << 0)) {
			ss << "R32";
		}
		if (_mask & (1 << 1)) {
			ss << "G32";
		}
		if (_mask & (1 << 2)) {
			ss << "B32";
		}
		if (_mask & (1 << 3)) {
			ss << "A32";
		}
		ss << '_';
		switch (_type)
		{
		case D3D_REGISTER_COMPONENT_UINT32:
			ss << "UINT";
			break;
		case D3D_REGISTER_COMPONENT_SINT32:
			ss << "SINT";
			break;
		case D3D_REGISTER_COMPONENT_FLOAT32:
			ss << "FLOAT";
			break;
		case D3D_REGISTER_COMPONENT_UNKNOWN:
		default:
			assert(false);
			break;
		}
		DXGI_FORMAT ret;
		auto result = EnumNameConverter::EnumValue<DXGI_FORMAT>(ss.str(), &ret);
		if (!result) {
			assert(false);
		}
		return ret;
	}
}

namespace D3D12FrameWork {
	bool ShaderRegisterDescs::Init(ID3D12ShaderReflection* _pRflc) {

		D3D12_SHADER_DESC sdesc = {};
		auto hr = _pRflc->GetDesc(&sdesc);
		if (FAILED(hr)) {
			assert(false);
			return false;
		}
		for (UINT i = 0; i < sdesc.BoundResources; i++) {
			D3D12_SHADER_INPUT_BIND_DESC bind_desc{};
			hr = _pRflc->GetResourceBindingDesc(i, &bind_desc);
			if (FAILED(hr)) {
				assert(false);
				return false;
			}
			//typeによって処理を分ける．構造体を作る部分は関数にして切り分ける．
			//キーが存在していればcontinue
			if (bind_desc.Type == D3D_SIT_CBUFFER) {
				if (BBindDesc.count(bind_desc.Name)) continue;

				BRegisterDesc.emplace_back(std::move(CreateRegisterDesc(bind_desc)));
				BBindDesc[bind_desc.Name] = ConstantBindDesc::None();
			}
			if (bind_desc.Type >= D3D_SIT_TBUFFER &&
				bind_desc.Type <= D3D_SIT_TEXTURE) {
				if (TBindDesc.count(bind_desc.Name)) continue;
				TRegisterDesc.emplace_back(std::move(CreateRegisterDesc(bind_desc)));
				TBindDesc[bind_desc.Name] = std::move(CreateTextureBindDesc(bind_desc));
			}
			if (bind_desc.Type == D3D_SIT_SAMPLER) {
				if (SBindDesc.count(bind_desc.Name)) continue;
				SRegisterDesc.emplace_back(std::move(CreateRegisterDesc(bind_desc)));
				SBindDesc[bind_desc.Name] = std::move(CreateSamplerBindDesc(bind_desc));
			}
		}

		//ConstantBufferについてさらに情報を得る
		for (UINT i = 0; i < sdesc.ConstantBuffers; i++) {
			//ここではcbの構造体情報まで掘り下げる
			auto pCB = _pRflc->GetConstantBufferByIndex(i);
			D3D12_SHADER_BUFFER_DESC b_desc;
			auto hr = pCB->GetDesc(&b_desc);
			if (FAILED(hr)) {
				assert(false);
				return false;
			}
			if (!BBindDesc.count(b_desc.Name)) {
				assert(false);
				return false;
			}
			//サイズ0ならまだ作られていない
			if (BBindDesc[b_desc.Name].Size != 0) continue;
			BBindDesc[b_desc.Name] = std::move(CreateConstantBindDesc(
				pCB, b_desc
			));
		}

		return true;
	}

	
	ShaderRegisterDescs::RegisterDesc ShaderRegisterDescs::CreateRegisterDesc(
		D3D12_SHADER_INPUT_BIND_DESC const& _bind_desc
	) {
		RegisterDesc ret{
			.Name = _bind_desc.Name,
			.BindPoint = _bind_desc.BindPoint,
			.BindCount = _bind_desc.BindCount,
			.uFlags = _bind_desc.uFlags,
			.Space = _bind_desc.Space
		};

		return ret;
	}

	
	ShaderRegisterDescs::TextureBindDesc ShaderRegisterDescs::CreateTextureBindDesc(
		D3D12_SHADER_INPUT_BIND_DESC const& bind_desc
	) {
		TextureDimension dim;
		switch (bind_desc.Dimension)
		{
		case D3D_SRV_DIMENSION_TEXTURE1D:
			dim = TextureDimension::Texture1D;
			break;
		case D3D_SRV_DIMENSION_TEXTURE2D:
			dim = TextureDimension::Texture2D;
			break;
		case D3D_SRV_DIMENSION_TEXTURE3D:
			dim = TextureDimension::Texture3D;
			break;
		default:
			assert(false && "未対応のテクスチャ");
			break;
		}
		TextureBindDesc ret{
			.Dimension = dim
		};
		return ret;
	}

	
	ShaderRegisterDescs::SamplerBindDesc ShaderRegisterDescs::CreateSamplerBindDesc(
		D3D12_SHADER_INPUT_BIND_DESC const& bind_desc
	) {
		SamplerBindDesc ret{
			.NumSamples = bind_desc.NumSamples
		};
		return ret;
	}

	
	ShaderRegisterDescs::ConstantBindDesc ShaderRegisterDescs::CreateConstantBindDesc(
		ID3D12ShaderReflectionConstantBuffer* _pCB,
		D3D12_SHADER_BUFFER_DESC const& b_desc
	) {
		auto ret = ConstantBindDesc{
			.Size = b_desc.Size,
			.VariableDesc = {}
		};
		for (UINT i = 0; i < b_desc.Variables; i++) {
			D3D12_SHADER_VARIABLE_DESC v_desc{};
			auto variable = _pCB->GetVariableByIndex(i);
			auto hr = variable->GetDesc(&v_desc);
			if (FAILED(hr)) {
				assert(false);
				return ConstantBindDesc::None();
			}
			ret.VariableDesc[v_desc.Name] =
				ConstantVariableDesc{
				.StartOffset = v_desc.StartOffset,
				.Size = v_desc.Size
			};
		}
		return ret;
	}

	bool ShaderOutputDescs::Init(ID3D12ShaderReflection* _pRflc) {
		D3D12_SHADER_DESC sdesc = {};
		auto hr = _pRflc->GetDesc(&sdesc);
		if (FAILED(hr)) {
			assert(false);
			return false;
		}
		m_elementDescs.reserve(sdesc.OutputParameters);
		for (UINT i = 0; i < sdesc.OutputParameters; i++) {
			D3D12_SIGNATURE_PARAMETER_DESC param_desc{};
			hr = _pRflc->GetOutputParameterDesc(i, &param_desc);
			if (FAILED(hr)) {
				assert(false);
				return false;
			}
			m_elementDescs.emplace_back(ShaderOutputElementDesc{
				.SemanticName = std::move(param_desc.SemanticName),
				.SemanticIndex = param_desc.SemanticIndex,
				.Register = param_desc.Register,
				.Format = MaskToFormat(param_desc.ComponentType, param_desc.Mask),
				});
		}
		return true;
	}

	bool ShaderInputDescs::Init(ID3D12ShaderReflection* _pRflc) {
		D3D12_SHADER_DESC sdesc = {};
		auto hr = _pRflc->GetDesc(&sdesc);
		if (FAILED(hr)) {
			assert(false);
			return false;
		}
		m_elementDescs.reserve(sdesc.InputParameters);
		constexpr UINT MAXREG = 15;//vbのスロットの最大値
		for (UINT i = 0; i < sdesc.InputParameters; i++) {
			D3D12_SIGNATURE_PARAMETER_DESC param_desc{};
			hr = _pRflc->GetInputParameterDesc(i, &param_desc);
			if (FAILED(hr)) {
				assert(false);
				return false;
			}
			auto sm_name = std::string(param_desc.SemanticName);
			unsigned int slotNum = 0;
			if (sm_name[0] == 'I')//rflcからregisterが読めないので独自の命名規則でslot情報を読む
			{
				auto num_end = sm_name.find('_');
				if (num_end == std::string::npos) continue;
				try
				{
					auto slotStr = sm_name.substr(1, num_end - 1);
					UINT slotNumtmp = std::stoul(slotStr);
					if (slotStr != std::to_string(slotNumtmp) ||
						slotNumtmp > MAXREG) throw std::out_of_range("invalid slot num");
					slotNum = slotNumtmp;
				}
				catch (const std::invalid_argument& e)
				{
					continue;
				}
				catch (const std::out_of_range& e) {
					throw e;
				}
			}
			m_elementDescs.emplace_back(ShaderInputElementDesc{
				.SemanticName = std::move(param_desc.SemanticName),
				.SemanticIndex = param_desc.SemanticIndex,
				.Register = slotNum,
				.Format = MaskToFormat(param_desc.ComponentType, param_desc.Mask),
				});
		}
		//slotごとにソート
		std::sort(m_elementDescs.begin(), m_elementDescs.end(), [](auto const& lhs, auto const& rhs) {
			return lhs.Register < rhs.Register;
			});
		return true;
	}
}

