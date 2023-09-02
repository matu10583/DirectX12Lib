#pragma once
#include <d3d12shader.h>
#include <d3dcompiler.h>
#include "dxc/dxcapi.h"
#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/ComPtr.h"
#include <tuple>


#pragma comment(lib, "dxcompiler")
namespace D3D12FrameWork {
	
	enum ShaderCompileOption
	{
		 SHADERCOMPILE_DEBUG                          =(1 << 0),
		 SHADERCOMPILE_SKIP_VALIDATION                =(1 << 1),
		 SHADERCOMPILE_SKIP_OPTIMIZATION              =(1 << 2),
		 SHADERCOMPILE_PACK_MATRIX_ROW_MAJOR          =(1 << 3),
		 SHADERCOMPILE_PACK_MATRIX_COLUMN_MAJOR       =(1 << 4),
//		 SHADERCOMPILE_PARTIAL_PRECISION              =(1 << 5),
//		 SHADERCOMPILE_FORCE_VS_SOFTWARE_NO_OPT       =(1 << 6),
//		 SHADERCOMPILE_FORCE_PS_SOFTWARE_NO_OPT       =(1 << 7),
//		 SHADERCOMPILE_NO_PRESHADER                   =(1 << 8),
		 SHADERCOMPILE_AVOID_FLOW_CONTROL             =(1 << 9),
		 SHADERCOMPILE_PREFER_FLOW_CONTROL            =(1 << 10),
		 SHADERCOMPILE_ENABLE_STRICTNESS              =(1 << 11),
		 SHADERCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY =(1 << 12),
		 SHADERCOMPILE_IEEE_STRICTNESS                =(1 << 13),
		 SHADERCOMPILE_OPTIMIZATION_LEVEL0            =(1 << 14),
		 SHADERCOMPILE_OPTIMIZATION_LEVEL1            =0,
		 SHADERCOMPILE_OPTIMIZATION_LEVEL2            =((1 << 14) | (1 << 15)),
		 SHADERCOMPILE_OPTIMIZATION_LEVEL3            =(1 << 15),
//		 SHADERCOMPILE_RESERVED16                     =(1 << 16),
//		 SHADERCOMPILE_RESERVED17                     =(1 << 17),
		 SHADERCOMPILE_WARNINGS_ARE_ERRORS            =(1 << 18)
	};

	extern const std::map<ShaderCompileOption, std::tuple<LPCWSTR, UINT>> CompileOptionTable;
	class IBlobObject;
	class ShaderCompiler
	{
	public:
		enum ShaderCompilerType
		{
			DXC=0,
			FXC=1
		};
		virtual bool CompileShader(
			std::filesystem::path _shaderPath,
			std::filesystem::path _compiledPath,
			std::string_view _entryPoint,
			std::string_view _shaderVersion,
			std::vector<ShaderCompileOption> const& _compileOption,
			IBlobObject** _ppShader,
			ID3D12ShaderReflection** _ppRflc=nullptr,
			bool isPreCompiled=false) = 0;
		static bool Create(ShaderCompilerType _type);
		static void Destroy() {
			if (m_instance != nullptr) {
				delete m_instance;
				m_instance = nullptr;
			}
		}
		static ShaderCompiler* Instance() { return m_instance; }
	private:
		static ShaderCompiler* m_instance;

	protected:
		virtual ~ShaderCompiler() = default;
		ShaderCompiler(ShaderCompiler const&) = default;
		ShaderCompiler() = default;
	};


	class DXCompiler: public ShaderCompiler
	{
		friend class ShaderCompiler;
	public:

		virtual bool CompileShader(
			std::filesystem::path _shaderPath,
			std::filesystem::path _compiledPath,
			std::string_view _entryPoint,
			std::string_view _shaderVersion,
			std::vector<ShaderCompileOption>const& _compileOption,
			IBlobObject** _ppShader,
			ID3D12ShaderReflection** _ppRflc,
			bool isPreCompiled)override;

	private:
		bool Init();
		void Term();
		DXCompiler(){}
		DXCompiler(DXCompiler const&){}
		~DXCompiler(){
			Term();
		}
		ComPtr<IDxcUtils> m_pUtils;
		ComPtr<IDxcCompiler3> m_pCompiler;
		ComPtr<IDxcContainerReflection> m_pContainerRefl;
		//ComPtr<IDxcIncludeHandler> m_pIncHandler;

		//コンパイルオプションの配列を後ろに漬けてく。
		void PushBackDxcArguments(ShaderCompileOption const* _opts, size_t optSize, IDxcCompilerArgs* _cargs);
	};

	class FXCompiler: public ShaderCompiler
	{
		friend class ShaderCompiler;
	public:

		virtual bool CompileShader(
			std::filesystem::path _shaderPath,
			std::filesystem::path _compiledPath,
			std::string_view _entryPoint,
			std::string_view _shaderVersion,
			std::vector<ShaderCompileOption>const& _compileOption,
			IBlobObject** _ppShader,
			ID3D12ShaderReflection** _ppRflc,
			bool isPreCompiled)override;

	private:
		bool Init();
		void Term();
		FXCompiler(){}
		FXCompiler(DXCompiler const&){}
		~FXCompiler(){
			Term();
		}

		UINT GetFxcFlag(std::vector<ShaderCompileOption>const& _opts);
	};
}


