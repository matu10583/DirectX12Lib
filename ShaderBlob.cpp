#include "D3D12FrameWork/ShaderBlob.h"
#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/ComPtr.h"
#include "D3D12FrameWork/DXCompiler.h"
#include <filesystem>
#include <tchar.h>
#include <fstream>
#include <ctype.h>

namespace D3D12FrameWork {


	ShaderBlob::ShaderBlob()
	{
	}
	ShaderBlob::~ShaderBlob() {

	}

	bool ShaderBlob::Init(
		std::wstring_view _shaderName,
		ShaderType _type,
		ID3D12ShaderReflection** _ppRflc,
		DX12Settings::ShaderVersion _version
	) {
		m_pShaderFileChecker.reset(new ShaderFileChecker(std::wstring(
			_shaderName.begin(), _shaderName.end()
		)));
		//とりあえずdebug用
		auto compArg = std::vector<ShaderCompileOption>();

		compArg.emplace_back(ShaderCompileOption::SHADERCOMPILE_DEBUG);
		compArg.emplace_back(ShaderCompileOption::SHADERCOMPILE_SKIP_OPTIMIZATION);

		std::string up_initial;
		std::string low_initial;
		switch (_type)
		{
		case D3D12FrameWork::ShaderBlob::PIXEL:
			up_initial = "P"; low_initial = "p";
			break;
		case D3D12FrameWork::ShaderBlob::VERTEX:
			up_initial = "V"; low_initial = "v";
			break;
		case D3D12FrameWork::ShaderBlob::COMPUTE:
			up_initial = "C"; low_initial = "c";
			break;
		case D3D12FrameWork::ShaderBlob::GEOMETRY:
			up_initial = "G"; low_initial = "g";
			break;
		case D3D12FrameWork::ShaderBlob::AMPLIFY:
			up_initial = "A"; low_initial = "a";
			break;
		case D3D12FrameWork::ShaderBlob::MESH:
			up_initial = "M"; low_initial = "m";
			break;
		case D3D12FrameWork::ShaderBlob::NONE:
			break;
		default:
			break;
		}
		std::string version;
		switch (_version)
		{
		case D3D12FrameWork::DX12Settings::VERSION_5_0:
			version = "5_0";
			break;
		case D3D12FrameWork::DX12Settings::VERSION_6_5:
			version = "6_5";
			break;
		case D3D12FrameWork::DX12Settings::VERSION_6_0:
			version = "6_0";
			break;
		default:
			break;
		}

		IBlobObject* pTmpBlob = nullptr;
		auto result = ShaderCompiler::Instance()->CompileShader(
			m_pShaderFileChecker->GetShaderPath(),
			m_pShaderFileChecker->GetCompiledPath(),
			up_initial+"Smain",
			low_initial+"s_"+version,
			compArg,
			&pTmpBlob,
			_ppRflc,
			m_pShaderFileChecker->IsPreCompiled()
		);
		m_pShaderBlob.reset(pTmpBlob);

		return result;
	}


	ShaderFileChecker::ShaderFileChecker(std::wstring _shaderName)
	{
		m_shaderPath = std::filesystem::path(
			DX12Settings::SHADER_PATH / (_shaderName + L".hlsl")
		);
		m_compiledPath = std::filesystem::path(
			DX12Settings::SHADER_PATH / (_shaderName + L".cso")
		);
	}

	ShaderFileChecker::~ShaderFileChecker()
	{
	}
	
	bool ShaderFileChecker::IsPreCompiled()
	{
		if (!std::filesystem::exists(m_compiledPath)) return false;

#if defined(DEBUG) || defined(_DEBUG)

		if (!std::filesystem::exists(m_shaderPath)) {
			assert(false);
		}
		auto comDate = std::filesystem::last_write_time(m_compiledPath).time_since_epoch();
		auto shdDate = std::filesystem::last_write_time(m_shaderPath).time_since_epoch();
		if (comDate < shdDate) {//shaderの更新日時より前にコンパイルしている．
			return false;
		}
		//hlsliも調べておく
		auto includePaths = CollectIncludePath();
		for (auto ipath : includePaths) {
			auto incShader = m_shaderPath.parent_path().append(ipath);
			if (!std::filesystem::exists(incShader)) {
				assert(false);
			}
			auto incDate = std::filesystem::last_write_time(incShader).time_since_epoch();
			if (comDate < incDate) {
				return false;
			}
		}
		//pdbも調べておく。
		//auto pdbPath = DX12Settings::SHADER_PDB_PATH / m_shaderPath.filename();
		//if (!std::filesystem::exists(pdbPath)) {
		//	return false;
		//}
		
#endif // DEBUG
		return true;
	}

	std::list<std::string> 
		ShaderFileChecker::CollectIncludePath() {
		std::list<std::string> ret;
		std::ifstream shFile(m_shaderPath);
		if (!shFile.is_open()) {
			assert(false);
			return ret;
		}

		std::string line;
		std::string includeMacro = "#include";
		while (std::getline(shFile,line) &&
			!std::isalpha(line[0]))
		{
			auto result = line.find(includeMacro);
			if (result == std::string::npos) continue;
			auto startPos = line.find_first_of('"', result);
			if (startPos == std::string::npos) continue;
			auto endPos = line.find_last_of('"');
			if (endPos <= startPos) continue;

			auto includePath = line.substr(startPos + 1, endPos - startPos - 1);
			ret.emplace_back(includePath);
		}

		return ret;
	}
}
