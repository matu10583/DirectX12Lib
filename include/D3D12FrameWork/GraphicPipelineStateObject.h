#pragma once
#include "D3D12FrameWork/ShaderSet.h"
#include "D3D12FrameWork/ComPtr.h"
#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/RPRegisterDescSet.h"
#include <d3d12.h>
#include <algorithm>


namespace D3D12FrameWork {
	class RootSignature;
	class D3Device;
	struct GraphicPipelineStateDesc
	{
		D3D12_RASTERIZER_DESC RasterizerState;
		D3D12_BLEND_DESC BlendState;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyType;
		DXGI_SAMPLE_DESC SampleDesc;
		D3D12_DEPTH_STENCIL_DESC DepthStencil;
	};






	class IPipelineStateObject {
	public:
		IPipelineStateObject() = default;
		virtual ~IPipelineStateObject() = default;
		virtual std::string_view GetName()const = 0;
		//このパイプラインにおけるデフォルトのバッファを設定する．
		virtual bool SetDefaultMaterial(std::string_view _buffName, uint32_t _rpIdx) = 0;
		virtual std::string_view GetDefaultMaterialName(uint32_t _rpIdx)const = 0;
		virtual RootSignature const* GetRootSignature()const = 0;
		virtual ID3D12PipelineState* GetPipelineState()const = 0;
		virtual RootParamRegisterDesc const& GetRPRegisterDesc(uint32_t _rpIdx)const = 0;
		virtual ShaderInputDescs const& GetInputDesc()const = 0;
		virtual ShaderOutputDescs const& GetOutputDesc()const = 0;
		virtual void AddRenderComponent(std::reference_wrapper<class RenderComponent const>) = 0;
		virtual bool RemoveRenderComponent(class RenderComponent const* const) = 0;
	};

	template<ShaderBlob::ShaderType... ST>
	class GraphicPipelineStateObject:public IPipelineStateObject
	{

	public:
		GraphicPipelineStateObject(std::string_view _name)
			: m_shaderSet()
			,m_name(_name.data()) {
		}
		DECLMOVECOPY(GraphicPipelineStateObject);

		bool PrepareRootSignature(class RootSignature* _rs);

		template<ShaderBlob::ShaderType T>
		bool PrepareShaderAsInput(std::wstring_view _shaderName) {
			ShaderBindDescs<ShaderRegisterDescs,ShaderInputDescs> tmp_desc;
			if (!m_shaderSet.InitShader<T, ShaderRegisterDescs, ShaderInputDescs>(_shaderName, &tmp_desc)) {
				assert(false);
				return false;
			}
			//これらの情報からマテリアル生成用の情報を整理する．
			auto const& input_desc = tmp_desc.GetDesc<ShaderInputDescs>();
			m_inputDesc = std::move(input_desc);
			auto const& reg_desc = tmp_desc.GetDesc<ShaderRegisterDescs>();
			return m_rootParamsRegDescs.Load<T>(reg_desc,
				m_pRefRootSignature->GetRootParamDescs());
		}

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
	

		bool EndPrepareAndInit( GraphicPipelineStateDesc const&, D3DDevice*);

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
		RootParamRegisterDesc const& GetRPRegisterDesc(uint32_t _rpIdx)const override{
			if (_rpIdx >=m_rootParamsRegDescs.Size()) {
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
				[&_renderComp](std::reference_wrapper<RenderComponent const>& _rc){
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
		ShaderInputDescs const& GetInputDesc()const override {
			return m_inputDesc;
		}
		ShaderOutputDescs const& GetOutputDesc()const override {
			return m_outputDesc;
		}
	private:
		ShaderSet<ST...> m_shaderSet;
		RPRegisterDescSet m_rootParamsRegDescs;
		RootSignature* m_pRefRootSignature;
		ShaderInputDescs m_inputDesc;
		ShaderOutputDescs m_outputDesc;
		std::string m_name;
		std::vector<std::string> m_defaultRpBuffNames;
		std::vector<std::reference_wrapper<RenderComponent const>> m_renderedComps;


		ComPtr<ID3D12PipelineState> m_pPSO;

	};
}

#include "D3D12FrameWork/details/GraphicPipelineStateObject_detail.h"
