#pragma once
#include "D3D12FrameWork/ComPtr.h"
#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/ShaderKind.h"
#include "D3D12FrameWork/ShaderBindInfo.h"
#include "D3D12FrameWork/ShaderBlob.h"
#include <d3d12.h>
#include <unordered_map>
#include <d3d12shader.h>
#include "D3D12FrameWork/Set.h"

namespace D3D12FrameWork {

	template<ShaderBlob::ShaderType... Args>
	class ShaderSet
	{

	public:

		
		ShaderSet()
			:m_shaderFlag(ShaderBlob::ShaderType::NONE)
			{}


		GETTERCONST(Set<ShaderBlob::ShaderType>, ShaderFlag, m_shaderFlag);

		template<ShaderBlob::ShaderType T, ShaderBindInfo... Descs>
		bool InitShader(std::wstring_view _shaderName,
			ShaderBindDescs<Descs...>* _outDescs) {
			Shader<T>& p_shader = std::get<Shader<T>>(m_shaders);
			if (!p_shader.Init(_shaderName,_outDescs)) {
				assert(false);
				return false;
			}
			m_shaderFlag |= T;
			return true;
		}
		template<ShaderBlob::ShaderType T>
		Shader<T> const& GetShader()const {
			return std::get<Shader<T>>(m_shaders);
		}



	private:
		std::tuple<Shader<Args>...> m_shaders;

		Set<ShaderBlob::ShaderType> m_shaderFlag;

	};
}

#include "D3D12FrameWork/details/ShaderSet.h"

