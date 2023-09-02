#include "D3D12FrameWork/Texture.h"
#include "D3D12FrameWork/D3DDevice.h"
#include "D3D12FrameWork/CommandList.h"
#include "DirectXTex/DirectXTex.h"
#include <d3d12.h>
#include <filesystem>

//stbLib
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace D3D12FrameWork {
	
	Texture::Texture()
		:m_Desc()
	{
		
	}

	bool Texture::Init(D3DDevice* dev, TextureDesc const& desc) {
		D3D12_HEAP_PROPERTIES prop = {};
		prop.Type = D3D12_HEAP_TYPE_DEFAULT;
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		prop.CreationNodeMask = 0;
		prop.VisibleNodeMask = 0;

		D3D12_RESOURCE_DESC resDesc = {};
		switch (desc.dimension)
		{
		case TextureDimension::Texture1D:
			resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
			break;
		case TextureDimension::Texture2D:
			resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			break;
		case TextureDimension::Texture3D:
			resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
			break;
		default:
			break;
		}
		resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		resDesc.Height = desc.height;
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resDesc.MipLevels = desc.miplevels;
		resDesc.SampleDesc.Count = desc.sampleCount;
		resDesc.SampleDesc.Quality = desc.sampleQuality;
		resDesc.Width = desc.width;
		resDesc.Alignment = 0;
		resDesc.DepthOrArraySize = desc.depthOrArraySize;

		//Depthのフォーマットはsrvを作るためにtypelessにする
		auto resFormat = desc.format;
		switch (resFormat)
		{
		case DXGI_FORMAT_D32_FLOAT:
			resFormat = DXGI_FORMAT_R32_TYPELESS;
			break;
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
			resFormat = DXGI_FORMAT_R32G8X24_TYPELESS;
			break;
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
			resFormat = DXGI_FORMAT_R24G8_TYPELESS;
			break;
		case DXGI_FORMAT_D16_UNORM:
			resFormat = DXGI_FORMAT_R16_TYPELESS;
			break;
		default:
			break;
		}

		D3D12_CLEAR_VALUE clVal = {};
		clVal.Format = resFormat;
		resDesc.Format = resFormat;
		switch (desc.type)
		{
		case TextureType::RenderTarget:
			memcpy(clVal.Color, desc.color, sizeof(clVal.Color));
			resDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			break;
		case TextureType::DepthBuffer:
			clVal.DepthStencil.Depth = desc.depthStencil.depth;
			clVal.DepthStencil.Stencil = desc.depthStencil.stencil;
			resDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			break;
		case TextureType::UnorderAccess:
			resDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		default:
			break;
		}
		
		auto hr = dev->GetDev()->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			desc.currentState,
			&clVal,
			IID_PPV_ARGS(m_pResource.ReleaseAndGetAddressOf())
		);
		if (FAILED(hr)) {
			assert(SUCCEEDED(hr));
			return false;
		}

		m_Desc = desc;
		return true;
	}

	bool 
		Texture::InitFromPNG(
		D3DDevice* _dev,
		CommandList* _cmdList,
		void const* pPng,
		size_t size,
		uint32_t mipL
	) {
		auto pScrImg = LoadPNG(
			pPng, size, mipL
		);
		if (!InitFromDXImage(_dev, _cmdList, pScrImg.get())) {
			return false;
		}
		return true;
	}

	unqPtr<DirectX::ScratchImage> 
		Texture::LoadPNG(void const* pPng, size_t size, uint32_t mipL) {
		if (!pPng) {
			return unqPtr<DirectX::ScratchImage>();
		}

		//png読み込み
		int width, height, bpp;
		auto pixels = stbi_loadf_from_memory(
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

		return std::move(image);
	}


	bool 
		Texture::InitFromDXImage(D3DDevice* dev,
			CommandList* cmdList,
			DirectX::ScratchImage const* image) {
		auto meta = image->GetMetadata();
		D3D12_HEAP_PROPERTIES heapProp = {};
		heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProp.CreationNodeMask = 0;
		heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProp.VisibleNodeMask = 0;

		D3D12_RESOURCE_DESC resDesc = {};
		resDesc.Alignment = 0;
		if (meta.dimension == DirectX::TEX_DIMENSION_TEXTURE3D) {
			resDesc.DepthOrArraySize = 
				static_cast<uint16_t>(meta.depth);
		}
		else {
			resDesc.DepthOrArraySize = 
				static_cast<uint16_t>(meta.arraySize);
		}
		resDesc.Dimension =
			static_cast<D3D12_RESOURCE_DIMENSION>(meta.dimension);
		resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		resDesc.Format = meta.format;
		resDesc.Height = meta.height;
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resDesc.MipLevels = meta.mipLevels;
		resDesc.SampleDesc.Count = 1;
		resDesc.SampleDesc.Quality = 0;
		resDesc.Width = meta.width;
		

		auto hr = dev->GetDev()->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(m_pResource.ReleaseAndGetAddressOf())
		);
		if (FAILED(hr)) {
			assert(SUCCEEDED(hr));
			return false;
		}

		memset(&m_Desc, 0, sizeof(TextureDesc));
		m_Desc.currentState = D3D12_RESOURCE_STATE_COPY_DEST;
		m_Desc.depthOrArraySize = resDesc.DepthOrArraySize;
		switch (resDesc.Dimension)
		{
		case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
			m_Desc.dimension = TextureDimension::Texture1D;
			break;
		case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
			m_Desc.dimension = TextureDimension::Texture2D;
			break;
		case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
			m_Desc.dimension = TextureDimension::Texture3D;
			break;
		default:
			break;
		}
		m_Desc.format = resDesc.Format;
		m_Desc.height = resDesc.Height;
		m_Desc.miplevels = resDesc.MipLevels;
		m_Desc.sampleCount = resDesc.SampleDesc.Count;
		m_Desc.sampleQuality = resDesc.SampleDesc.Quality;
		m_Desc.width = resDesc.Width;		
		
		//リソースのコピー
		ID3D12Resource* pSrcRes;
		if (!UploadAndCopyImage(dev, cmdList, image, &pSrcRes)) {
			assert(false);
			return false;
		}
		//コマンド実行までは消さない
		dev->PendingRelease<ID3D12Resource>(pSrcRes);

		return true;
	}

	bool Texture::InitFromSwapChain(D3DDevice* _dev, SwapChain* _sc, uint32_t _bufferIdx) {
		auto hr = _sc->GetDxSwapChain()->GetBuffer(_bufferIdx,
			IID_PPV_ARGS(m_pResource.ReleaseAndGetAddressOf()));
		if (FAILED(hr)) {
			assert(SUCCEEDED(hr));
			return false;
		}

		auto desc = m_pResource->GetDesc();
		m_Desc.currentState = D3D12_RESOURCE_STATE_PRESENT;
		m_Desc.depthOrArraySize = desc.DepthOrArraySize;
		switch (desc.Dimension)
		{
		case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
			m_Desc.dimension = TextureDimension::Texture1D;
			break;
		case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
			m_Desc.dimension = TextureDimension::Texture2D;
			break;
		case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
			m_Desc.dimension = TextureDimension::Texture3D;
			break;
		default:
			break;
		}
		m_Desc.format = desc.Format;
		m_Desc.height = desc.Height;
		m_Desc.miplevels = desc.MipLevels;
		m_Desc.sampleCount = desc.SampleDesc.Count;
		m_Desc.sampleQuality = desc.SampleDesc.Quality;
		m_Desc.type = TextureType::RenderTarget;
		m_Desc.width = desc.Width;

		return true;
	}

	bool
		Texture::UploadAndCopyImage(
			D3DDevice* dev,
			CommandList* cmdList,
			DirectX::ScratchImage const* image,
			ID3D12Resource** ppResource
		) {
		const DirectX::TexMetadata& meta = image->GetMetadata();

		uint32_t numSubRes = static_cast<uint32_t>(meta.arraySize * meta.mipLevels);
		std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> fp(numSubRes);
		std::vector<uint32_t> numRows(numSubRes);
		std::vector<uint64_t> rowSize(numSubRes);
		uint64_t totalSize;


		D3D12_RESOURCE_DESC resDesc = {};
		switch (m_Desc.dimension)
		{
		case TextureDimension::Texture1D:
			resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
			break;
		case TextureDimension::Texture2D:
			resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			break;
		case TextureDimension::Texture3D:
			resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
			break;
		default:
			break;
		}
		resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		resDesc.Height = m_Desc.height;
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resDesc.MipLevels = m_Desc.miplevels;
		resDesc.SampleDesc.Count = m_Desc.sampleCount;
		resDesc.SampleDesc.Quality = m_Desc.sampleQuality;
		resDesc.Width = m_Desc.width;
		resDesc.Alignment = 0;
		resDesc.DepthOrArraySize = m_Desc.depthOrArraySize;
		resDesc.Format = m_Desc.format;
		//resDesc = m_pResource->GetDesc();

		//リソースの配列情報を取得
		dev->GetDev()->GetCopyableFootprints(
			&resDesc, 0,
			numSubRes, 0,
			fp.data(),
			numRows.data(),
			rowSize.data(),
			&totalSize
		);

		//アップロード用のオブジェクト
		D3D12_HEAP_PROPERTIES heapProp = {};
		heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProp.CreationNodeMask = 0;
		heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapProp.VisibleNodeMask = 0;

		resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resDesc.Height = 1;
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resDesc.Width = totalSize;
		resDesc.Alignment = 0;
		resDesc.DepthOrArraySize = 1;
		resDesc.Format = DXGI_FORMAT_UNKNOWN;
		resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ID3D12Resource* pUploadSrc;
		auto hr = dev->GetDev()->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&pUploadSrc)
		);
		if (FAILED(hr)) {
			assert(SUCCEEDED(hr));
			return false;
		}

		//リソースをマップしてコピー
		uint8_t* pMappedSrc;
		hr = pUploadSrc->Map(0, nullptr, reinterpret_cast<void**>(&pMappedSrc));
		if (FAILED(hr)) {
			pUploadSrc->Release();
			pUploadSrc = nullptr;
			assert(SUCCEEDED(hr));
			return false;
		}

		for (uint32_t s = 0; s < meta.arraySize; s++) {
			for (uint32_t m = 0; m < meta.mipLevels; m++) {
				//何番目のリソースか
				size_t i = s * meta.mipLevels + m;
				if (rowSize[i] > (size_t)-1) {
					pUploadSrc->Release();
					pUploadSrc = nullptr;
					assert(SUCCEEDED(hr));
					return false;
				}

				D3D12_MEMCPY_DEST cpyDstData;
				cpyDstData.pData = pMappedSrc + fp[i].Offset;
				cpyDstData.RowPitch = fp[i].Footprint.RowPitch;
				cpyDstData.SlicePitch = fp[i].Footprint.RowPitch * numRows[i];
				DirectX::Image const* pImg = image->GetImage(m, 0, s);
				if (rowSize[i] == fp[i].Footprint.RowPitch) {
					memcpy(cpyDstData.pData, pImg->pixels, pImg->rowPitch * numRows[i]);
				}
				else if (rowSize[i] < fp[i].Footprint.RowPitch) {//256Alignmentによりサイズが異なる
					uint8_t* pSrcPix = pImg->pixels;
					uint8_t* pDstDat = reinterpret_cast<uint8_t*>(cpyDstData.pData);
					for (uint32_t r = 0; r < numRows[i];
						r++, pSrcPix += pImg->rowPitch, pDstDat += fp[i].Footprint.RowPitch) {
						memcpy(pDstDat, pSrcPix, rowSize[i]);
					}
				}
			}
		}
		pUploadSrc->Unmap(0, nullptr);

		//コピー
		for (uint32_t i = 0; i < numSubRes; i++) {
			D3D12_TEXTURE_COPY_LOCATION src, dst;
			src.pResource = pUploadSrc;
			src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			src.PlacedFootprint = fp[i];
			dst.pResource = m_pResource.Get();
			dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			dst.SubresourceIndex = i;
			cmdList->GetList()->CopyTextureRegion(&dst,0,0,0,&src,nullptr);
		}

		*ppResource = pUploadSrc;
		return true;
	}

}
