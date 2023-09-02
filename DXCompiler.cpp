#include "D3D12FrameWork/DXCompiler.h"
#include <fstream>
#include <list>
#include <vector>
#include <ranges>
#include <string_view>
#include "D3D12FrameWork/ShaderBlob.h"
#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/File.h"
#include "D3D12FrameWork/util/wcharUtil.h"
#include "dxc/DxilContainer/DxilContainer.h"


namespace {
	bool CheckErrorBlob(IDxcBlob* _errBlob) {
		if (_errBlob->GetBufferSize() == 0) return true;
#if defined(DEBUG) || defined(_DEBUG)
		std::string errstr;
		errstr.resize(_errBlob->GetBufferSize());
		std::copy_n((char*)_errBlob->GetBufferPointer(),
			_errBlob->GetBufferSize(),
			errstr.begin());
		OutputDebugStringA(errstr.c_str());
#endif // define
		return false;
	}
}

namespace D3D12FrameWork{
	const std::map<ShaderCompileOption, std::tuple<LPCWSTR, UINT>>
		CompileOptionTable =
	{
		{ShaderCompileOption::SHADERCOMPILE_DEBUG,
			{L"-Zi -Qembed_debug -WX -Zss", D3DCOMPILE_DEBUG}},
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
		
		ComPtr<IDxcResult> compiledBuff{};
		//shader情報とreflが入るはず
		IDxcBlobEncoding* pcompiledShaderObj{};
		//シェーダーコンパイルとリフレクション取得
		if (isPreCompiled) {
			//.csoを読んでくる
			File csoFile;
			csoFile.ReadFile(_compiledPath, true);
			auto hr = m_pUtils->CreateBlob(csoFile.GetData(), csoFile.GetSize(), DXC_CP_ACP,
				&pcompiledShaderObj);
			if (FAILED(hr)) {
				assert(false);
				return false;
			}
		}
		else {
			//shaderコードの読み込み
			ComPtr<IDxcBlobEncoding> sourceBlob{};
			auto hr = m_pUtils->LoadFile(_shaderPath.c_str(), nullptr,
				sourceBlob.ReleaseAndGetAddressOf());

			DxcBuffer srcBuff{
				.Ptr = sourceBlob->GetBufferPointer(),
				.Size = sourceBlob->GetBufferSize(),
				.Encoding = 0u,
			};

			//set compiler option
			constexpr int MaxWCharSize = 30;
			auto entryPSize = GetWCharBuffSizeFromChar(_entryPoint);
			if (MaxWCharSize < entryPSize) {
				std::stringstream oss;
				oss << "Shaderのエントリポイント名は" << MaxWCharSize << "文字以下にしてください！";
				throw new std::length_error(oss.str());
			}
			wchar_t entryPoint[MaxWCharSize];
			if (!StrToWStr(_entryPoint, entryPoint, MaxWCharSize)) assert(false);
			std::wstring_view wEP(entryPoint, entryPSize);

			auto shaderVSize = GetWCharBuffSizeFromChar(_shaderVersion);
			if (MaxWCharSize < shaderVSize) {
				std::stringstream oss;
				oss << "Shaderのバージョン名は" << MaxWCharSize << "文字以下じゃないの？";
				throw new std::length_error(oss.str());
			}
			wchar_t shaderVersion[MaxWCharSize];
			if (!StrToWStr(_shaderVersion, shaderVersion, MaxWCharSize)) assert(false);
			std::wstring_view wSV(shaderVersion, shaderVSize);

			auto shader_dir = std::filesystem::canonical(
				_shaderPath.parent_path());
			LPCWSTR constArgs[] = {
				L"-I",
				shader_dir.c_str()
			};

			ComPtr<IDxcCompilerArgs> pCmpArgs{};
			hr = m_pUtils->BuildArguments(
				_shaderPath.c_str(),
				wEP.data(),
				wSV.data(),
				constArgs,
				ARRAYSIZE(constArgs),
				nullptr,
				0,
				pCmpArgs.ReleaseAndGetAddressOf()
			);

			//指定されたコンパイルオプションを漬ける。
			PushBackDxcArguments(_compileOption.data(), _compileOption.size(), pCmpArgs.Get());

			//filesystemからinclude
			ComPtr<IDxcIncludeHandler> defaultInc;
			hr = m_pUtils->CreateDefaultIncludeHandler(defaultInc.ReleaseAndGetAddressOf());
			if (FAILED(hr)) {
				assert(false);
				return false;
			}

			hr = m_pCompiler->Compile(
				&srcBuff,
				pCmpArgs->GetArguments(),
				pCmpArgs->GetCount(),
				defaultInc.Get(),
				IID_PPV_ARGS(compiledBuff.ReleaseAndGetAddressOf())
			);

			if (FAILED(hr)) {
				assert(false);
				return false;
			}

			ComPtr<IDxcBlobUtf16> outputName{};
			ComPtr<IDxcBlob> errBlob{};
			hr = compiledBuff->GetOutput(DXC_OUT_ERRORS, 
				IID_PPV_ARGS(errBlob.ReleaseAndGetAddressOf()),
				outputName.ReleaseAndGetAddressOf());
			if (FAILED(hr) ||
				!CheckErrorBlob(errBlob.Get())) {
				assert(false);
				return false;
			}		

			//コンパイルした結果を取得
			ComPtr<IDxcBlob> compiledShaderBlob{};
			hr = compiledBuff->GetResult(compiledShaderBlob.ReleaseAndGetAddressOf());
			if (FAILED(hr)) {
				assert(false);
				return false;
			}

#if defined(DEBUG) | defined(_DEBUG)
			hr = m_pContainerRefl->Load(compiledShaderBlob.Get());
			//pdb名を取得
			UINT32 debugNameIndex;
			hr = m_pContainerRefl->FindFirstPartKind(hlsl::DFCC_ShaderDebugName, &debugNameIndex);
			ComPtr<IDxcBlob> pPdbName;
			hr = m_pContainerRefl->GetPartContent(debugNameIndex, pPdbName.ReleaseAndGetAddressOf());
			//デバッグ情報の取得
			UINT32 debugInfoIndex;
			hr = m_pContainerRefl->FindFirstPartKind(hlsl::DFCC_ShaderDebugInfoDXIL, &debugInfoIndex);
			ComPtr<IDxcBlob> pPdb;
			hr = m_pContainerRefl->GetPartContent(debugInfoIndex, pPdb.ReleaseAndGetAddressOf());
			//pdbファイル名の変換
			auto pDebugNameData = reinterpret_cast<hlsl::DxilShaderDebugName const*>(
				pPdbName->GetBufferPointer());
			auto pName = reinterpret_cast<char const*>(pDebugNameData + 1);
			
			//ファイルセーブ処理
			//auto pdbShaderPath = DX12Settings::SHADER_PDB_PATH / _shaderPath.filename();
			//if (!std::filesystem::exists(pdbShaderPath)) {
			//	std::filesystem::create_directory(pdbShaderPath);
			//}
			if (!File::WriteFile(DX12Settings::SHADER_PDB_PATH / pName, pPdb->GetBufferSize(),
				reinterpret_cast<uint8_t*>(pPdb->GetBufferPointer()))) {
				printf("%sのpdbの保存に失敗しました．", _shaderPath.string().c_str());
			}
#endif


			//pdbを除いた情報を生成
			ComPtr<IDxcContainerBuilder> pDxcContainerBuilder;
			::DxcCreateInstance(CLSID_DxcContainerBuilder,
				IID_PPV_ARGS(pDxcContainerBuilder.ReleaseAndGetAddressOf()));
			pDxcContainerBuilder->Load(compiledShaderBlob.Get());
#if defined(NDEBUG)
			pDxcContainerBuilder->RemovePart(hlsl::DFCC_ShaderDebugInfoDXIL);
#endif // !defined(_DEBUG)|| defined(DEBUG)
			ComPtr<IDxcOperationResult> pstrippedResult;
			pDxcContainerBuilder->SerializeContainer(pstrippedResult.ReleaseAndGetAddressOf());
			pstrippedResult->GetResult((IDxcBlob**)&pcompiledShaderObj);
			//ファイルセーブ処理
			if (!File::WriteFile(_compiledPath, pcompiledShaderObj->GetBufferSize(),
				reinterpret_cast<uint8_t*>(pcompiledShaderObj->GetBufferPointer()))) {
				printf("%sのコンパイルデータの保存に失敗しました．", _shaderPath.string().c_str());
			}

			
		}//end precompiled

		//shader reflectionの取得
		if (_ppRflc != nullptr) {
			auto hr = m_pContainerRefl->Load(pcompiledShaderObj);
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
		
		//shader情報を返す
		*_ppShader = new BlobObject<IDxcBlobEncoding>(pcompiledShaderObj);
		
		return true;
	}
	
	void
		DXCompiler::PushBackDxcArguments(ShaderCompileOption const* _opts, size_t optSize, 
			IDxcCompilerArgs* _cargs) {
		for (int i = 0; i < optSize; i++) {
			LPCWSTR o = std::get<ShaderCompiler::ShaderCompilerType::DXC>(
				CompileOptionTable.at(_opts[i]));
			auto vo = std::wstring_view(o);
			if (vo.find_first_of(L" ") == std::wstring_view::npos) {
				LPCWSTR tmpptrs[] = { vo.data() };
				_cargs->AddArguments(tmpptrs, 1);
				continue;
			}
			for (auto const& arg : std::views::split(vo, L' ')) {
				//newしたくないけどなんか静的確保で多めにとっても無理なので
				//まあコンパイルなんてほぼ実行時には行われないしいいでしょ
				auto argChr = new WCHAR[arg.size() + 1];
				std::memcpy(reinterpret_cast<void*>(argChr),
					reinterpret_cast<void const*>(arg.data()),
					arg.size()*sizeof(WCHAR));
				argChr[arg.size()] = L'\0';
				_cargs->AddArguments((LPCWSTR*)(&argChr), 1);
				delete[] argChr;
			}
		}
		return;
	}

	void DXCompiler::Term() {
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
