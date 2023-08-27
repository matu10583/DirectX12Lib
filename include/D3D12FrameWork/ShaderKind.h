#pragma once
#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/ComPtr.h"
#include "D3D12FrameWork/ShaderBlob.h"
#include "D3D12FrameWork/ShaderBindInfo.h"
#include <concepts>

namespace D3D12FrameWork {
	template<ShaderBlob::ShaderType T>
	class Shader_Impl
	{
	public:
		Shader_Impl()
			:m_shader()
		{}
		virtual ~Shader_Impl() {
			Term();
		}
		DECLMOVECOPY(Shader_Impl);
		IBlobObject const* const GetBlob()const {
			return m_shader.GetShaderBlob();
		}

		GETTERPTR(IBlobObject, ShaderBlob, m_shader.GetShaderBlob());


		template<ShaderBindInfo... Descs>
		bool Init(std::wstring_view _shaderName,
			ShaderBindDescs<Descs...>* _outDescs) {
			ComPtr<ID3D12ShaderReflection> pRflc;
			if (!m_shader.Init(
				_shaderName,
				T,
				pRflc.ReleaseAndGetAddressOf(),
				ShaderBlob::ShaderVersion::VERSION_5_0
			)) {
				assert(false);
				return false;
			}
			if (!_outDescs->Init(pRflc.Get())) {
				assert(false);
				return false;
			}

			return true;
		}
	private:
		void Term() {

		}
		ShaderBlob m_shader;

		
	};
	
	template<ShaderBlob::ShaderType T>
	class Shader :public Shader_Impl<T> {
	};
	//template<ShaderBlob::ShaderType T>
	//class Shader<T> :public Shader_Impl<T, ShaderRegisterDescs> {
	//};
	//template<ShaderBlob::ShaderType T>
	//requires (T==ShaderBlob::VERTEX)
	//class Shader<T> :public Shader_Impl<T, ShaderRegisterDescs, ShaderInputDescs> {
	//};
	//template<ShaderBlob::ShaderType T>
	//requires (T == ShaderBlob::PIXEL)
	//class Shader<T> :public Shader_Impl<T, ShaderRegisterDescs, ShaderOutputDescs> {
	//};
}



