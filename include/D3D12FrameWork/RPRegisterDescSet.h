#pragma once
#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/ShaderSet.h"
#include "D3D12FrameWork/ComPtr.h"

namespace D3D12FrameWork {
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

	class RPRegisterDescSet
	{
	public:
		size_t Size() const {
			return m_rootParamsRegDescs.size();
		}
		auto const& Get(uint32_t idx) const {
			assert(idx < Size());
			return m_rootParamsRegDescs[idx];
		}
		void Resize(size_t size) {
			
		}

		bool Init(std::vector<RootSignature::RSRootParameterDesc> const& _rsRpdesc) {
			m_rootParamsRegDescs.resize(_rsRpdesc.size());
			for (int i = 0; i < _rsRpdesc.size(); i++) {
				auto& rpRegdesc = m_rootParamsRegDescs[i];
				auto const& rpDesc = _rsRpdesc[i];

				rpRegdesc.RangeDescs.resize(rpDesc.Ranges.size());
			}
			return true;
		}

		template<ShaderBlob::ShaderType T>
		bool Load(ShaderRegisterDescs const& _regDescs,
			std::vector<RootSignature::RSRootParameterDesc> const& rpDesc) {


			std::vector<std::pair<size_t, RootSignature::RSRootParameterDesc const*>> allRP;
			std::vector<std::pair<size_t, RootSignature::RSRootParameterDesc const*>> svisibleRP;
			for (auto i = 0; i < rpDesc.size(); i++) {
				if (rpDesc[i].VisibleShader == T) svisibleRP.emplace_back(std::pair(i, &(rpDesc[i])));
				if (rpDesc[i].VisibleShader == ShaderBlob::NONE) allRP.emplace_back(std::pair(i, &(rpDesc[i])));
			}
			for (auto const paRpd : allRP) {
				auto const& aRpd = *(paRpd.second);
				auto& aSBnd = m_rootParamsRegDescs[paRpd.first].SamplerBindDesc;
				auto& aBBnd = m_rootParamsRegDescs[paRpd.first].ConstantBindDesc;
				auto& aTBnd = m_rootParamsRegDescs[paRpd.first].TextureBindDesc;
				//Globalだけresizeして余分に持っておく．initが終わったらいらない要素に関しては消す
				//m_rootParamsRegDescs[paRpd.first].RangeDescs.resize(aRpd.Ranges.size());
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
				//m_rootParamsRegDescs[psRpd.first].RangeDescs.resize(sRpd.Ranges.size());
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
	private:
		std::vector<RootParamRegisterDesc> m_rootParamsRegDescs;
	};
}


