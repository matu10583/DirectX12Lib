#include "D3D12FrameWork/DXCompiler.h"
#include <fstream>
#include <list>
#include <vector>
#include "D3D12FrameWork/ShaderBlob.h"
#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/File.h"


namespace D3D12FrameWork{
	const std::map<ShaderCompileOption, std::tuple<LPCWSTR, UINT>>
		CompileOptionTable =
	{
		{ShaderCompileOption::SHADERCOMPILE_DEBUG,
			{L"-Zi", D3DCOMPILE_DEBUG}},
		{ShaderCompileOption::SHADERCOMPILE_SKIP_VALIDATION,
			{L"-Vd", D3DCOMPILE_SKIP_VALIDATION}},
		{ShaderCompileOption::SHADERCOMPILE_SKIP_OPTIMIZATION,
			{L"-Od",D3DCOMPILE_SKIP_OPTIMIZATION}},
		{ShaderCompileOption::SHADERCOMPILE_PACK_MATRIX_ROW_MAJOR,
			{L"-Zpr",D3DCOMPILE_PACK_MATRIX_ROW_MAJOR}},
		{ShaderCompileOption::SHADERCOMPILE_PACK_MATRIX_COLUMN_MAJOR,
			{L"-Zpc",D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR}},
		//{ShaderCompileOption::SHADERCOMPILE_PARTIAL_PRECISION,
		//	{L"",D3DCOMPILE_PARTIAL_PRECISION}},
		//{ShaderCompileOption::SHADERCOMPILE_FORCE_VS_SOFTWARE_NO_OPT,
		//	{L"",D3DCOMPILE_FORCE_VS_SOFTWARE_NO_OPT}},
		//{ShaderCompileOption::SHADERCOMPILE_FORCE_PS_SOFTWARE_NO_OPT,
		//	{L"",D3DCOMPILE_FORCE_PS_SOFTWARE_NO_OPT}},
		//{ShaderCompileOption::SHADERCOMPILE_NO_PRESHADER,
		//	{L"",D3DCOMPILE_NO_PRESHADER}},
		{ShaderCompileOption::SHADERCOMPILE_AVOID_FLOW_CONTROL,
			{L"-Gfa",D3DCOMPILE_AVOID_FLOW_CONTROL}},
		{ShaderCompileOption::SHADERCOMPILE_PREFER_FLOW_CONTROL,
			{L"-Gfp",D3DCOMPILE_PREFER_FLOW_CONTROL}},
		{ShaderCompileOption::SHADERCOMPILE_ENABLE_STRICTNESS,
			{L"-Ges",D3DCOMPILE_ENABLE_STRICTNESS}},
		{ShaderCompileOption::SHADERCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY ,
			{L"-Gec",D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY }},
		{ShaderCompileOption::SHADERCOMPILE_IEEE_STRICTNESS  ,
			{L"-Gis",D3DCOMPILE_IEEE_STRICTNESS  }},
		{ShaderCompileOption::SHADERCOMPILE_OPTIMIZATION_LEVEL0,
			{L"-O0",D3DCOMPILE_OPTIMIZATION_LEVEL0}},
		{ShaderCompileOption::SHADERCOMPILE_OPTIMIZATION_LEVEL1,
			{L"-O1",D3DCOMPILE_OPTIMIZATION_LEVEL1}},
		{ShaderCompileOption::SHADERCOMPILE_OPTIMIZATION_LEVEL2,
			{L"-O2",D3DCOMPILE_OPTIMIZATION_LEVEL2}},
		{ShaderCompileOption::SHADERCOMPILE_OPTIMIZATION_LEVEL3,
			{L"-O3",D3DCOMPILE_OPTIMIZATION_LEVEL3}},
		//{ShaderCompileOption::SHADERCOMPILE_RESERVED16,
		//	{L"",D3DCOMPILE_RESERVED16}},
		//{ShaderCompileOption::SHADERCOMPILE_RESERVED17,
		//	{L"",D3DCOMPILE_RESERVED17}},
		{ShaderCompileOption::SHADERCOMPILE_WARNINGS_ARE_ERRORS,
			{L"-WX",D3DCOMPILE_WARNINGS_ARE_ERRORS}}

	};

	ShaderCompiler* ShaderCompiler::m_instance = nullptr;

	bool ShaderCompiler::Create(ShaderCompilerType _type) {
		if (m_instance != nullptr) return false;

		bool succeededInit = true;
		
		if (_type == ShaderCompilerType::DXC) {
			auto tmp = new DXCompiler();
			if (!tmp->Init()) succeededInit = false;
			m_instance = tmp;
		}
		else if (_type == ShaderCompilerType::FXC) {
			auto tmp = new FXCompiler();
			if (!tmp->Init()) succeededInit = false;
			m_instance = tmp;
		}
		else {
			assert(false);
		}

		if (!succeededInit) {
			delete m_instance;
			m_instance = nullptr;
			return false;
		}

		return true;
	}


	bool DXCompiler::Init() {
		auto hr = ::DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_pUtils));
		RETURNIFFAILED(hr);
		hr = ::DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_pCompiler));
		RETURNIFFAILED(hr);
		/*hr = m_pUtils->CreateDefaultIncludeHandler(&m_pIncHandler);
		RETURNIFFAILED(hr);*/
		hr = ::DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(&m_pContainerRefl));
		RETURNIFFAILED(hr);
		hr = ::DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&m_pLib));
		RETURNIFFAILED(hr);

		return true;
	}

	bool DXCompiler::CompileShader(
		std::filesystem::path _shaderPath,
		std::filesystem::path _compiledPath,
		std::string_view _entryPoint,
		std::string_view _shaderVersion,
		std::vector<ShaderCompileOption>const& _compileOption,
		IBlobObject** _ppShader,
		ID3D12ShaderReflection** _ppRflc,
		bool isPreCompiled
	) {
		IDxcBlob* compiledShaderBlob{};
		//シェーダーコンパイルとリフレクション取得
		if (isPreCompiled) {
			ID3DBlob* compiledRes{};
			auto hr = D3DReadFileToBlob(_compiledPath.c_str(), &compiledRes);
			if (FAILED(hr)) {
				assert(false);
				compiledRes->Release();
				compiledRes = nullptr;
				return false;
			}
			*_ppShader = new BlobObject<ID3DBlob>(compiledRes);
			if (_ppRflc != nullptr) {
				hr = D3DReflect(compiledRes->GetBufferPointer(),
					compiledRes->GetBufferSize(),
					IID_PPV_ARGS(_ppRflc));

				if (FAILED(hr)) {
					assert(false);
					compiledRes->Release();
					compiledRes = nullptr;
					return false;
				}
			}

			
		}
		else {
			ComPtr<IDxcBlobEncoding> sourceBlob{};
			auto hr = m_pUtils->LoadFile(_shaderPath.c_str(), nullptr,
				sourceBlob.ReleaseAndGetAddressOf());

			DxcBuffer srcBuff{
				.Ptr = sourceBlob->GetBufferPointer(),
				.Size = sourceBlob->GetBufferSize(),
				.Encoding = 0u,
			};

			//set compiler option
			std::vector<LPCWSTR> cmpArgs = GetDxcArguments(_compileOption);
			cmpArgs.emplace_back(L"-E");
			cmpArgs.emplace_back(StrToWStr(_entryPoint).c_str());
			cmpArgs.emplace_back(L"-T");
			cmpArgs.emplace_back(StrToWStr(_shaderVersion).c_str());

			ComPtr<IDxcResult> compiledBuff{};
			hr = m_pCompiler->Compile(
				&srcBuff,
				cmpArgs.data(),
				static_cast<uint32_t>(cmpArgs.size()),
				nullptr,
				IID_PPV_ARGS(compiledBuff.ReleaseAndGetAddressOf())
			);
			if (FAILED(hr)) {
				assert(false);
				return false;
			}
			hr = compiledBuff->GetResult(&compiledShaderBlob);
			if (FAILED(hr)) {
				assert(false);
				compiledShaderBlob->Release();
				compiledShaderBlob = nullptr;
				return false;
			}
			//ファイルセーブ処理
			if (!File::WriteFile(_compiledPath, compiledShaderBlob->GetBufferSize(),
				reinterpret_cast<uint8_t*>(compiledShaderBlob->GetBufferPointer()))) {
				printf("%sのコンパイルデータの保存に失敗しました．", _shaderPath.string().c_str());
			}
			
		}

		*_ppShader= new BlobObject<IDxcBlob>(compiledShaderBlob);
		//shader reflectionの取得
		if (_ppRflc != nullptr) {
			auto hr = m_pContainerRefl->Load(compiledShaderBlob);
			if (FAILED(hr)) {
				assert(false);
				return false;
			}
			UINT shdIdx = 0;
			hr = m_pContainerRefl->FindFirstPartKind(DXC_PART_DXIL, &shdIdx);
			if (FAILED(hr)) {
				assert(false);
				return false;
			}
			hr = m_pContainerRefl->GetPartReflection(shdIdx,
				IID_PPV_ARGS(_ppRflc));
			if (FAILED(hr)) {
				assert(false);
				return false;
			}
		}
		
		return true;
	}
	
	std::vector<LPCWSTR>
		DXCompiler::GetDxcArguments(std::vector<ShaderCompileOption>const& _opts) {
		std::vector<LPCWSTR> ret;
		ret.clear();
		for (int i = 0; i < _opts.size(); i++) {
			LPCWSTR o = std::get<ShaderCompiler::ShaderCompilerType::DXC>(
				CompileOptionTable.at(_opts[i]));
			ret.emplace_back(o);
		}
		return ret;
	}

	void DXCompiler::Term() {
		//SAFERELEASE(m_pIncHandler);
		SAFERELEASE(m_pCompiler);
		SAFERELEASE(m_pUtils);
	}

	bool FXCompiler::Init() {
		return true;
	}
	void FXCompiler::Term() {

	}
	bool FXCompiler::CompileShader(
		std::filesystem::path _shaderPath,
		std::filesystem::path _compiledPath,
		std::string_view _entryPoint,
		std::string_view _shaderVersion,
		std::vector<ShaderCompileOption>const& _compileOption,
		IBlobObject** _ppShader,
		ID3D12ShaderReflection** _ppRflc,
		bool isPreCompiled
	) {
		ID3DBlob* compiledRes{};
		if (isPreCompiled) {
			auto hr = D3DReadFileToBlob(_compiledPath.c_str(), &compiledRes);
			if (FAILED(hr)) {
				assert(false);
				if(compiledRes)compiledRes->Release();
				compiledRes = nullptr;
				return false;
			}
		}
		else {
			ComPtr<ID3DBlob> errBlob{};
			auto flag1 = GetFxcFlag(_compileOption);
			auto hr = D3DCompileFromFile(
				_shaderPath.c_str(),
				nullptr,
				D3D_COMPILE_STANDARD_FILE_INCLUDE,
				_entryPoint.data(),
				_shaderVersion.data(),
				flag1,
				0,
				&compiledRes,
				errBlob.ReleaseAndGetAddressOf()
			);
			if (FAILED(hr)) {
				if (errBlob)
				{
					OutputDebugStringA((char*)errBlob->GetBufferPointer());
					errBlob->Release();
				}
				assert(false);
				if(compiledRes)compiledRes->Release();
				compiledRes = nullptr;
				return false;
			}

			//ファイルセーブ処理
			if (!File::WriteFile(_compiledPath, compiledRes->GetBufferSize(),
				reinterpret_cast<uint8_t*>(compiledRes->GetBufferPointer()))) {
				printf("%sのコンパイルデータの保存に失敗しました．", _shaderPath.string().c_str());
			}
		}
		*_ppShader = new BlobObject<ID3DBlob>(compiledRes);
		
		if(_ppRflc!=nullptr){
			auto hr = D3DReflect(compiledRes->GetBufferPointer(),
				compiledRes->GetBufferSize(),
				IID_PPV_ARGS(_ppRflc));

			if (FAILED(hr)) {
				assert(false);
				compiledRes->Release();
				compiledRes = nullptr;
				return false;
			}
		}

		return true;
	}

	UINT FXCompiler::GetFxcFlag(std::vector<ShaderCompileOption>const& _opts) {
		UINT ret = 0;
		for (auto& opt : _opts) {
			ret |=
				std::get<ShaderCompiler::ShaderCompilerType::FXC>(CompileOptionTable.at(opt));
		}
		return ret;
	}

}
