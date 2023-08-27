#include "D3D12FrameWork/ImageLoader.h"
//stbLib
#include <stb/stb_image.h>
#include <fstream>
#include <DirectXTex/DirectXTex.h>



namespace D3D12FrameWork {
	std::unordered_map<std::string, unqPtr<DirectX::ScratchImage>>
		ImageLoader::m_imgDataMap = {};

	std::unordered_map<std::string, std::function<unqPtr<DirectX::ScratchImage>(uint8_t const*, size_t const, uint32_t const)>>
		ImageLoader::m_loaders2dxImg = {
		{".png", ImageLoader::LoadPng2DxImg}
	};

	DirectX::ScratchImage const*
		ImageLoader::Load2DxImg(std::filesystem::path const& file_path,
		uint32_t mipL) {
		if (!std::filesystem::exists(file_path)) {
			assert(false);
			return nullptr;
		}
		if (m_imgDataMap.count(file_path.string())) {
			return m_imgDataMap[file_path.string()].get();
		}
		std::ifstream ifst(file_path, std::ios::binary);
		ifst.seekg(0, std::ios::end);
		size_t const fileSize = ifst.tellg();
		ifst.seekg(0, std::ios::beg);

		std::vector<uint8_t> buff(fileSize);
		ifst.read(reinterpret_cast<char*>(buff.data()), fileSize);
		ifst.close();
		//別スレッドにしたい
		m_imgDataMap[file_path.string()] = m_loaders2dxImg[file_path.extension().string()](
			buff.data(), fileSize, mipL
			);
		return m_imgDataMap[file_path.string()].get();
	}

	unqPtr<DirectX::ScratchImage>
		ImageLoader::LoadPng2DxImg(uint8_t const* pPng, size_t const size, uint32_t const mipL) {
		if (!pPng) {
			return unqPtr<DirectX::ScratchImage>();
		}

		//png読み込み
		int width, height, bpp;
		auto pixels = stbi_load_from_memory(
			reinterpret_cast<stbi_uc const*>(pPng),
			static_cast<int>(size),
			&width, &height,
			&bpp, 0
		);
		
		if (!pixels || (bpp != 3 && bpp != 4)) {
			return unqPtr<DirectX::ScratchImage>();
		}

		//DXTex形式に変換
		unqPtr<DirectX::ScratchImage> image =
			std::make_unique<DirectX::ScratchImage>();
		auto hr = image->Initialize2D(
			DXGI_FORMAT_R8G8B8A8_UNORM,
			width, height, 1, 1
		);
		if (FAILED(hr)) {
			return unqPtr<DirectX::ScratchImage>();
		}
		auto src = pixels;
		auto dst = image->GetPixels();
		if (bpp == 3) {
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < height; x++) {
					dst[0] = src[0];
					dst[1] = src[1];
					dst[2] = src[2];
					dst[3] = 0xff;
					src += 3;
					dst += 4;
				}
			}
		}
		else {
			memcpy(dst, src, width * height * bpp);
		}
		

		stbi_image_free(pixels);

		//ミップマップ作製
		if (mipL != 1) {
			unqPtr<DirectX::ScratchImage> mip_img =
				std::make_unique<DirectX::ScratchImage>();
			//levelが0の場合1x1までの完全なミップマップが作製される．
			DirectX::GenerateMipMaps(
				*image->GetImage(0, 0, 0),
				DirectX::TEX_FILTER_CUBIC | DirectX::TEX_FILTER_FORCE_NON_WIC,
				0, *mip_img
			);
			image.swap(mip_img);
		}
		//DirectX::SaveToDDSFile(image->GetImages(), image->GetImageCount(), image->GetMetadata(),
		//	DirectX::DDS_FLAGS_NONE, L"test_save.dds");

		return std::move(image);
	}

}

