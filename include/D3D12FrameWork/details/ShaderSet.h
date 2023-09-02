#include "D3D12FrameWork/ShaderSet.h"
#include "D3D12FrameWork/RootSignature.h"
#include<initializer_list>
#include <algorithm>

namespace D3D12FrameWork {
	/*template<ShaderBlob::ShaderType... Args>
	ShaderRegisterDescs ShaderSet<Args...>::CreateGlobalRegisterDesc(const UINT _maxNumGlbRegister) {
		std::vector<ShaderRegisterDescs> sr_descs;
		sr_descs.emplace_back(GetShader<ShaderBlob::VERTEX>().GetShaderDesc<ShaderRegisterDescs>());
		sr_descs.emplace_back(GetShader<ShaderBlob::PIXEL>().GetShaderDesc<ShaderRegisterDescs>());

		std::unordered_map<std::string, ShaderRegisterDescs::ConstantRegisterDesc> bReg;
		std::unordered_map<std::string, ShaderRegisterDescs::TextureRegisterDesc> tReg;
		std::unordered_map<std::string, ShaderRegisterDescs::SamplerRegisterDesc> sReg;
		
		for (auto& d : sr_descs) {
			std::copy_if(d.GetBRegister().begin(), d.GetBRegister().end(), std::inserter(bReg,bReg.end()),
				[bReg, _maxNumGlbRegister](auto const& pair) {
					return !bReg.count(pair.first) &&
						pair.second.Register.BindPoint < _maxNumGlbRegister;
				});
			std::copy_if(d.GetTRegister().begin(), d.GetTRegister().end(), std::inserter(tReg, tReg.end()),
				[tReg, _maxNumGlbRegister](auto const& pair) {
					return !tReg.count(pair.first) &&
						pair.second.Register.BindPoint < _maxNumGlbRegister;
				});
			std::copy_if(d.GetSRegister().begin(), d.GetSRegister().end(), std::inserter(sReg, sReg.end()),
				[sReg, _maxNumGlbRegister](auto const& pair) {
					return !sReg.count(pair.first) &&
						pair.second.Register.BindPoint < _maxNumGlbRegister;
				});
		}
		return ShaderRegisterDescs(std::move(bReg), std::move(tReg), std::move(sReg));
		
	}*/

}
