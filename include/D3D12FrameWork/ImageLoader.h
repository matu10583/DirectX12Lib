#pragma once
#include <unordered_map>
#include <functional>
#include <DirectXTex/DirectXTex.h>
#include <filesystem>
#include "D3D12FrameWork/ComPtr.h"
#include "D3D12FrameWork/Common.h"
namespace D3D12FrameWork {
	//フライウェイトにした
	class ImageLoader
	{
	public:
		static DirectX::ScratchImage const* Load2DxImg(std::filesystem::path const& file_path,
			uint32_t mipL);
	private:
		ImageLoader(){}
		~ImageLoader() {}
		DECLMOVECOPY(ImageLoader);
		static std::unordered_map<std::string, std::function<unqPtr<DirectX::ScratchImage>(uint8_t const*, size_t const, uint32_t const)>>
			m_loaders2dxImg;
		static unqPtr<DirectX::ScratchImage> LoadPng2DxImg(uint8_t const* pPng, size_t const size, uint32_t const mipL);
		static std::unordered_map<std::string, unqPtr<DirectX::ScratchImage>> m_imgDataMap;
	};


}