#pragma once
#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/Texture.h"
#include <unordered_map>
#include <d3d12shader.h>
#include <tuple>


namespace D3D12FrameWork {

	class ShaderRegisterDescs {
	public:
		struct RegisterDesc {
			std::string Name={};
			UINT BindPoint=0;
			UINT BindCount=0;
			UINT uFlags=0;
			UINT Space=0;
		};
		struct ConstantVariableDesc
		{
			UINT StartOffset;
			UINT Size;
		};
		struct ConstantBindDesc
		{
			UINT Size=0;
			std::unordered_map<std::string, ConstantVariableDesc> VariableDesc={};
			static ConstantBindDesc None() {
				return ConstantBindDesc{ 0 };
			}
		};
		//struct ConstantRegisterDesc
		//{
		//	RegisterDesc Register={};
		//	ConstantBindDesc ConstantBuffer={};

		//};
		struct TextureBindDesc
		{
			TextureDimension Dimension={};
		};
		//struct TextureRegisterDesc
		//{
		//	RegisterDesc Register={};
		//	TextureBindDesc Texture={};
		//};
		struct SamplerBindDesc
		{
			UINT NumSamples=0;
		};
		//struct SamplerRegisterDesc
		//{
		//	RegisterDesc Register={};
		//	SamplerBindDesc Sampler={};
		//};

		ShaderRegisterDescs()
			:BBindDesc(0)
			, SBindDesc(0)
			, TBindDesc(0) {}

		~ShaderRegisterDescs() = default;
		DECLMOVECOPY(ShaderRegisterDescs);

		bool Init(ID3D12ShaderReflection* _pRflc);


		std::unordered_map<std::string, ConstantBindDesc> BBindDesc;
		std::unordered_map<std::string, TextureBindDesc> TBindDesc;
		std::unordered_map<std::string, SamplerBindDesc> SBindDesc;
		std::vector<RegisterDesc> BRegisterDesc;
		std::vector<RegisterDesc> TRegisterDesc;
		std::vector<RegisterDesc> SRegisterDesc;

	private:
		ConstantBindDesc CreateConstantBindDesc(
			ID3D12ShaderReflectionConstantBuffer* _pCB,
			D3D12_SHADER_BUFFER_DESC const&
		);

		RegisterDesc CreateRegisterDesc(
			D3D12_SHADER_INPUT_BIND_DESC const&
		);
		TextureBindDesc CreateTextureBindDesc(
			D3D12_SHADER_INPUT_BIND_DESC const&
		);
		SamplerBindDesc CreateSamplerBindDesc(
			D3D12_SHADER_INPUT_BIND_DESC const&
		);
	};

	class ShaderInputDescs
	{
	public:
		struct ShaderInputElementDesc
		{
			std::string SemanticName;
			UINT SemanticIndex;
			UINT Register;
			DXGI_FORMAT Format;
		};
		ShaderInputDescs()
			:m_elementDescs(){}
		DECLMOVECOPY(ShaderInputDescs);
		bool Init(ID3D12ShaderReflection* _pRflc);
		ShaderInputElementDesc const& GetElement(UINT idx)const {
			return m_elementDescs[idx];
		}
		size_t GetSize()const {
			return m_elementDescs.size();
		}

	private:
		std::vector<ShaderInputElementDesc> m_elementDescs;

	};
	class ShaderOutputDescs {
	public:
		struct ShaderOutputElementDesc
		{
			std::string SemanticName;
			UINT SemanticIndex;
			UINT Register;
			DXGI_FORMAT Format;
		};
		ShaderOutputDescs(){}
		DECLMOVECOPY(ShaderOutputDescs);
		ShaderOutputElementDesc const& GetElement(UINT idx)const {
			return m_elementDescs[idx];
		}
		size_t GetSize()const {
			return m_elementDescs.size();
		}
		bool Init(ID3D12ShaderReflection* _pRflc);
	private:
		std::vector<ShaderOutputElementDesc> m_elementDescs;
	};



	template<typename T>
	concept ShaderBindInfo = FindType<T, ShaderRegisterDescs, ShaderInputDescs, ShaderOutputDescs>::value;
	template<ShaderBindInfo... SDescs>
	class ShaderBindDescs
	{
	public:
		ShaderBindDescs() = default;
		~ShaderBindDescs() = default;
		DECLMOVECOPY(ShaderBindDescs);

		template<ShaderBindInfo U>
		U const& GetDesc()const {
			return std::get<U>(m_SDescs);
		}


		bool Init(ID3D12ShaderReflection* _pRflc) {
			return InitDescs<SDescs...>(_pRflc);
		}

	private:
		template<ShaderBindInfo... S>
		bool InitDescs(ID3D12ShaderReflection* _pRflc) {
			return (std::get<S>(m_SDescs).Init(_pRflc) && ...);
		}
		std::tuple<SDescs...> m_SDescs;


	};

}
