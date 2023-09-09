#pragma once
#include "D3D12FrameWork/GraphicPipelineStateObject.h"
#include "D3D12FrameWork/ShaderSet.h"
#include "D3D12FrameWork/ComPtr.h"
#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/RPRegisterDescSet.h"
#include <d3d12.h>
#include <algorithm>

namespace D3D12FrameWork {

template<ShaderBlob::ShaderType... ST>
class MSPipelineStateObject :
    public IPipelineStateObject
{
	template<typename T, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE ObjT>
	class alignas(void*)
		StateParam {
	public:
		StateParam()
			:Type(ObjT)
			, Value(T()) {}
		StateParam(T const& val)
			:Type(ObjT)
			, Value(val) {}

		StateParam& operator=(T& const value) {
			Type = ObjT;
			Value = value;
			return *this;
		}
	private:
		D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type;
		T Value;
	};
#define PSST(x) D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_##x
	using SP_ROOT_SIGNATURE = StateParam<ID3D12RootSignature*, PSST(ROOT_SIGNATURE)>;
	using SP_AS = StateParam<D3D12_SHADER_BYTECODE, PSST(AS)>;
	using SP_MS = StateParam<D3D12_SHADER_BYTECODE, PSST(MS)>;
	using SP_PS = StateParam<D3D12_SHADER_BYTECODE, PSST(PS)>;
	using SP_BLEND = StateParam<D3D12_BLEND_DESC, PSST(BLEND)>;
	using SP_RASTERIZER = StateParam<D3D12_RASTERIZER_DESC, PSST(RASTERIZER)>;
	using SP_DEPTH_STENCIL = StateParam<D3D12_DEPTH_STENCIL_DESC, PSST(DEPTH_STENCIL)>;
	using SP_SAMPLE_MASK = StateParam<UINT, PSST(SAMPLE_MASK)>;
	using SP_SAMPLE_DESC = StateParam<DXGI_SAMPLE_DESC, PSST(SAMPLE_DESC)>;
	using SP_RT_FORMAT = StateParam<D3D12_RT_FORMAT_ARRAY, PSST(RENDER_TARGET_FORMATS)>;
	using SP_DS_FORMAT = StateParam<DXGI_FORMAT, PSST(DEPTH_STENCIL_FORMAT)>;
	using SP_FLAGS = StateParam<D3D12_PIPELINE_STATE_FLAGS, PSST(FLAGS)>;
#undef PSST
	struct MeshShaderPipelineStateDesc
	{
		SP_ROOT_SIGNATURE RootSignature;
		SP_AS AS;
		SP_MS MS;
		SP_PS PS;
		SP_BLEND Blend;
		SP_RASTERIZER Rasterizer;
		SP_DEPTH_STENCIL DepthStencil;
		SP_SAMPLE_MASK SampleMask;
		SP_SAMPLE_DESC SampleDesc;
		SP_RT_FORMAT RTFormats;
		SP_DS_FORMAT DSFormat;
		SP_FLAGS Flags;
	};

public:
	MSPipelineStateObject(std::string_view _name)
		: m_shaderSet()
		, m_name(_name.data()) {
	}
	DECLMOVECOPY(MSPipelineStateObject);

	bool PrepareRootSignature(class RootSignature* _rs);


	template<ShaderBlob::ShaderType T>
	bool PrepareShaderAsOutput(std::wstring_view _shaderName) {
		ShaderBindDescs<ShaderRegisterDescs, ShaderOutputDescs> tmp_desc;
		if (!m_shaderSet.InitShader<T, ShaderRegisterDescs, ShaderOutputDescs>(_shaderName, &tmp_desc)) {
			assert(false);
			return false;
		}
		//これらの情報からマテリアル生成用の情報を整理する．
		auto const& output_desc = tmp_desc.GetDesc<ShaderOutputDescs>();
		m_outputDesc = std::move(output_desc);
		auto const& reg_desc = tmp_desc.GetDesc<ShaderRegisterDescs>();
		return m_rootParamsRegDescs.Load<T>(reg_desc,
			m_pRefRootSignature->GetRootParamDescs());
	}

	template<ShaderBlob::ShaderType T>
	bool PrepareShader(std::wstring_view _shaderName) {
		ShaderBindDescs<ShaderRegisterDescs> tmp_desc;
		if (!m_shaderSet.InitShader<T, ShaderRegisterDescs>(_shaderName, &tmp_desc)) {
			assert(false);
			return false;
		}
		//これらの情報からマテリアル生成用の情報を整理する．
		auto const& reg_desc = tmp_desc.GetDesc<ShaderRegisterDescs>();
		return m_rootParamsRegDescs.Load<T>(reg_desc,
			m_pRefRootSignature->GetRootParamDescs());
	}


	bool EndPrepareAndInit(GraphicPipelineStateDesc const&, D3DDevice*);

	std::string_view GetName()const {
		return m_name;
	}

	bool SetDefaultMaterial(std::string_view _buffName, uint32_t _rpIdx) override {
		if (_rpIdx >= m_defaultRpBuffNames.size()) {
			return false;
		}
		m_defaultRpBuffNames[_rpIdx] = std::string(_buffName.data());
		return true;
	}
	std::string_view GetDefaultMaterialName(uint32_t _rpIdx)const override {
		if (_rpIdx >= m_defaultRpBuffNames.size()) {
			assert(false);
			return {};
		}
		return m_defaultRpBuffNames[_rpIdx];
	}
	RootSignature const* GetRootSignature()const override {
		return m_pRefRootSignature;
	}
	RootParamRegisterDesc const& GetRPRegisterDesc(uint32_t _rpIdx)const override {
		if (_rpIdx >= m_rootParamsRegDescs.Size()) {
			assert(false);
			//あー困ります困ります．返すものがありませんー．
		}
		return m_rootParamsRegDescs.Get(_rpIdx);
	}

	void AddRenderComponent(std::reference_wrapper<class RenderComponent const> _renderComp)override {
		m_renderedComps.emplace_back(_renderComp);
	}
	bool RemoveRenderComponent(class RenderComponent const* const _renderComp)override {
		auto it = std::find_if(m_renderedComps.begin(), m_renderedComps.end(),
			[&_renderComp](std::reference_wrapper<RenderComponent const>& _rc) {
				return &(_rc.get()) == _renderComp;
			});
		if (it != m_renderedComps.end()) {
			m_renderedComps.erase(it);
			return true;
		}
		return false;
	}

	ID3D12PipelineState* GetPipelineState()const override {
		return m_pPSO.Get();
	}
	ShaderInputDescs const& GetInputDesc()const override {//空です。
		return ShaderInputDescs();
	}
	ShaderOutputDescs const& GetOutputDesc()const override {
		return m_outputDesc;
	}

private:
	ShaderSet<ST...> m_shaderSet;
	RPRegisterDescSet m_rootParamsRegDescs;
	RootSignature* m_pRefRootSignature;
	ShaderOutputDescs m_outputDesc;
	std::string m_name;
	std::vector<std::string> m_defaultRpBuffNames;
	std::vector<std::reference_wrapper<RenderComponent const>> m_renderedComps;


	ComPtr<ID3D12PipelineState> m_pPSO;
};

}

#include "D3D12FrameWork/details/MSPipelineState_detail.h"

