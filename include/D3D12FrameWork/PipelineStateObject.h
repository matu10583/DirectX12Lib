#pragma once
#include "D3D12FrameWork/ShaderSet.h"
#include "D3D12FrameWork/ComPtr.h"
#include "D3D12FrameWork/Common.h"
#include <d3d12.h>
#include <algorithm>


namespace D3D12FrameWork {
	class RootSignature;
	class D3Device;
	struct PipelineStateDesc
	{
		D3D12_RASTERIZER_DESC RasterizerState;
		D3D12_BLEND_DESC BlendState;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyType;
		DXGI_SAMPLE_DESC SampleDesc;
	};


	struct DescriptorRangeRegisterDesc
	{
		std::vector<ShaderRegisterDescs::RegisterDesc> Registers;
	};

	struct RootParamRegisterDesc
	{
		std::unordered_map<std::string, ShaderRegisterDescs::ConstantBindDesc> ConstantBindDesc;
		std::unordered_map<std::string, ShaderRegisterDescs::TextureBindDesc> TextureBindDesc;
		std::unordered_map<std::string, ShaderRegisterDescs::SamplerBindDesc> SamplerBindDesc;
		std::vector<DescriptorRangeRegisterDesc> RangeDescs;
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
	class PipelineStateObject:public IPipelineStateObject
	{

	public:
		PipelineStateObject(std::string_view _name)
			: m_shaderSet()
			,m_name(_name.data()) {
		}
		DECLMOVECOPY(PipelineStateObject);

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
			return InitRPDescs<T>(reg_desc);
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
			return InitRPDescs<T>(reg_desc);
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
			return InitRPDescs<T>(reg_desc);
		}
	

		bool EndPrepareAndInit( PipelineStateDesc const&&, D3DDevice*);

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
			if (_rpIdx >=m_rootParamsRegDescs.size()) {
				assert(false);
				//あー困ります困ります．返すものがありませんー．
			}
			return m_rootParamsRegDescs[_rpIdx];
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
		std::vector<RootParamRegisterDesc> m_rootParamsRegDescs;
		RootSignature* m_pRefRootSignature;
		ShaderInputDescs m_inputDesc;
		ShaderOutputDescs m_outputDesc;
		std::string m_name;
		std::vector<std::string> m_defaultRpBuffNames;
		std::vector<std::reference_wrapper<RenderComponent const>> m_renderedComps;


		ComPtr<ID3D12PipelineState> m_pPSO;

		template<ShaderBlob::ShaderType T>
		bool InitRPDescs(ShaderRegisterDescs const& _regDescs) {
			auto const& rpDesc = m_pRefRootSignature->GetRootParamDescs();


			std::vector<std::pair<size_t, RootSignature::RSRootParameterDesc const*>> allRP;
			std::vector<std::pair<size_t, RootSignature::RSRootParameterDesc const*>> svisibleRP;
			for (auto i = 0; i < rpDesc.size();i++) {
				if (rpDesc[i].VisibleShader == T) svisibleRP.emplace_back(std::pair(i, &(rpDesc[i])));
				if (rpDesc[i].VisibleShader == ShaderBlob::NONE) allRP.emplace_back(std::pair( i, &(rpDesc[i])));
			}
			for (auto const paRpd : allRP) {
				auto const& aRpd = *(paRpd.second);
				auto& aSBnd = m_rootParamsRegDescs[paRpd.first].SamplerBindDesc;
				auto& aBBnd = m_rootParamsRegDescs[paRpd.first].ConstantBindDesc;
				auto& aTBnd = m_rootParamsRegDescs[paRpd.first].TextureBindDesc;
				//Globalだけresizeして余分に持っておく．initが終わったらいらない要素に関しては消す
				m_rootParamsRegDescs[paRpd.first].RangeDescs.resize(aRpd.Ranges.size());
				if (aRpd.Ranges.size() != 0 &&
					aRpd.Ranges[0].Type == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER) {
					for (int i = 0; i < aRpd.Ranges.size(); i++) {
						auto& sReg = m_rootParamsRegDescs[paRpd.first].RangeDescs[i];
						auto& range = aRpd.Ranges[i];
						for (auto const& reg : _regDescs.SRegisterDesc) {
							if (range.BaseShaderRegister <= reg.BindPoint &&
								reg.BindPoint + reg.BindCount <= range.BaseShaderRegister + range.NumDescriptors) {
								if (std::find_if(sReg.Registers.begin(), sReg.Registers.end(),
									[&reg](const auto& r) {return r.Name == reg.Name; }) == sReg.Registers.end()) {
									sReg.Registers.emplace_back(reg);
								}
								if (!aSBnd.count(reg.Name))aSBnd[reg.Name] =
									std::move(_regDescs.SBindDesc.at(reg.Name));
							}
						}
					}
				}
				else {//cbvsrvuav
					for (int i = 0; i < aRpd.Ranges.size(); i++) {
						if (aRpd.Ranges[i].Type == D3D12_DESCRIPTOR_RANGE_TYPE_CBV) {
							auto& range = aRpd.Ranges[i];
							auto& bReg = m_rootParamsRegDescs[paRpd.first].RangeDescs[i];
							for (auto const& reg : _regDescs.BRegisterDesc) {
								if (range.BaseShaderRegister <= reg.BindPoint &&
									reg.BindPoint + reg.BindCount <= range.BaseShaderRegister + range.NumDescriptors) {
									if (std::find_if(bReg.Registers.begin(), bReg.Registers.end(),
										[&reg](const auto& r) {return r.Name == reg.Name; }) == bReg.Registers.end()) {
										bReg.Registers.emplace_back(reg);
									}
									if (!aBBnd.count(reg.Name))aBBnd[reg.Name] =
										std::move(_regDescs.BBindDesc.at(reg.Name));
								}
							}
						}
						if (aRpd.Ranges[i].Type == D3D12_DESCRIPTOR_RANGE_TYPE_SRV) {
							auto& range = aRpd.Ranges[i];
							auto& tReg = m_rootParamsRegDescs[paRpd.first].RangeDescs[i];
							for (auto const& reg : _regDescs.TRegisterDesc) {
								if (range.BaseShaderRegister <= reg.BindPoint &&
									reg.BindPoint + reg.BindCount <= range.BaseShaderRegister + range.NumDescriptors) {
									if (std::find_if(tReg.Registers.begin(), tReg.Registers.end(),
										[&reg](const auto& r) {return r.Name == reg.Name; }) == tReg.Registers.end()) {
										tReg.Registers.emplace_back(reg);
									}
									if (!aTBnd.count(reg.Name))aTBnd[reg.Name] =
										std::move(_regDescs.TBindDesc.at(reg.Name));
								}
							}
						}

					}
				}
			}
			for (auto const psRpd : svisibleRP) {
				auto& sSBnd = m_rootParamsRegDescs[psRpd.first].SamplerBindDesc;
				auto& sBBnd = m_rootParamsRegDescs[psRpd.first].ConstantBindDesc;
				auto& sTBnd = m_rootParamsRegDescs[psRpd.first].TextureBindDesc;
				auto const& sRpd = *(psRpd.second);
				m_rootParamsRegDescs[psRpd.first].RangeDescs.resize(sRpd.Ranges.size());
				if (sRpd.Ranges.size() != 0 &&
					sRpd.Ranges[0].Type == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER) {//samplerのrpは全部smp
					for (int i = 0; i < sRpd.Ranges.size(); i++) {
						auto& range = sRpd.Ranges[i];
						for (auto const& reg : _regDescs.SRegisterDesc) {
							if (range.BaseShaderRegister <= reg.BindPoint &&
								reg.BindPoint + reg.BindCount <= range.BaseShaderRegister + range.NumDescriptors) {
								m_rootParamsRegDescs[psRpd.first].RangeDescs[i]
									.Registers.emplace_back(reg);
								if (!sSBnd.count(reg.Name))sSBnd[reg.Name] =
									std::move(_regDescs.SBindDesc.at(reg.Name));
							}
						}
					}
				}
				else {//cbvsrvuav
					for (int i = 0; i < sRpd.Ranges.size(); i++) {
						if (sRpd.Ranges[i].Type == D3D12_DESCRIPTOR_RANGE_TYPE_CBV) {
							auto& range = sRpd.Ranges[i];
							for (auto const& reg : _regDescs.BRegisterDesc) {
								if (range.BaseShaderRegister <= reg.BindPoint &&
									reg.BindPoint + reg.BindCount <= range.BaseShaderRegister + range.NumDescriptors) {
									m_rootParamsRegDescs[psRpd.first].RangeDescs[i]
										.Registers.emplace_back(reg);
									if (!sBBnd.count(reg.Name))sBBnd[reg.Name] =
										std::move(_regDescs.BBindDesc.at(reg.Name));
								}
							}
						}
						if (sRpd.Ranges[i].Type == D3D12_DESCRIPTOR_RANGE_TYPE_SRV) {
							auto& range = sRpd.Ranges[i];
							for (auto const& reg : _regDescs.TRegisterDesc) {
								if (range.BaseShaderRegister <= reg.BindPoint &&
									reg.BindPoint + reg.BindCount <= range.BaseShaderRegister + range.NumDescriptors) {
									m_rootParamsRegDescs[psRpd.first].RangeDescs[i]
										.Registers.emplace_back(reg);
									if (!sTBnd.count(reg.Name))sTBnd[reg.Name] =
										std::move(_regDescs.TBindDesc.at(reg.Name));
								}
							}
						}

					}
				}
			}
			return true;
		}
	};
}

#include "D3D12FrameWork/details/PipelineStateObject_detail.h"
