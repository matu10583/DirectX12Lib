#pragma once
#include <string>
#include <wchar.h>
//#include <d3dcompiler.h>

#include "D3D12FrameWork/ComPtr.h"
#include "D3D12FrameWork/Common.h"
#include <list>
#include <dxcapi.h>
#include <vector>
#include <d3d12shader.h>
#include "D3D12FrameWork/ShaderBindInfo.h"
#include "D3D12FrameWork/DX12Settings.h"

//#pragma comment(lib, "d3dcompiler.lib")
//#pragma comment(lib, "dxguid.lib")

namespace D3D12FrameWork {
	class IBlobObject {
	public:
		IBlobObject() = default;
		virtual ~IBlobObject() = default;
		virtual SIZE_T GetBufferSize()const= 0;
		virtual LPVOID GetBufferPointer()const = 0;
	};
	template<typename T>
	class BlobObject :public IBlobObject {
	public:
		BlobObject(T* _pBlob)
			:m_pBlob(_pBlob){}
		~BlobObject() {
			m_pBlob->Release();
			m_pBlob = nullptr;
		}
		virtual SIZE_T GetBufferSize()const override { return m_pBlob->GetBufferSize(); }
		virtual LPVOID GetBufferPointer()const override { return m_pBlob->GetBufferPointer(); }

	private:
		T* m_pBlob;
	};



	class ShaderFileChecker
	{
	public:
		ShaderFileChecker(std::wstring _shaderName);
		~ShaderFileChecker();

		auto& GetShaderPath()const {
			return m_shaderPath;
		}
		auto& GetCompiledPath()const {
			return m_compiledPath;
		}

		bool IsPreCompiled();
	private:
		std::filesystem::path m_shaderPath;
		std::filesystem::path m_compiledPath;
		std::list<std::string>
			CollectIncludePath();
	};

	class ShaderBlob
	{
	public:
		enum ShaderType
		{
			NONE = 0,
			PIXEL = 1 << 0,
			VERTEX = 1 << 1,
			COMPUTE = 1 << 2,
			GEOMETRY = 1 << 3,
		};


		ShaderBlob();
		~ShaderBlob();
		bool Init(std::wstring_view _shaderName, ShaderType _type, 
			ID3D12ShaderReflection** _ppRflc=nullptr,
			DX12Settings::ShaderVersion _version= DX12Settings::SHADER_VERSION);

		

		GETTERPTR(IBlobObject, ShaderBlob, m_pShaderBlob.get());

	private:
		unqPtr<ShaderFileChecker> m_pShaderFileChecker;
		unqPtr<IBlobObject> m_pShaderBlob;


	};

}



