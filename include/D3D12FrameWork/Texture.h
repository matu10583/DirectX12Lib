#pragma once
#include <DirectXTex/DirectXTex.h>
#include <d3d12.h>
#include "D3D12FrameWork/ComPtr.h"
#include "D3D12FrameWork/Common.h"
//ID3D12ResourceをTexture用にラップ
namespace D3D12FrameWork {
	class IResource {
	public:
		virtual ID3D12Resource* GetResource()const = 0;
	};

	class D3DDevice;
	class CommandList;
	class SwapChain;

	//テクスチャの次元
	enum class TextureDimension
	{
		Texture1D = 0,
		Texture2D,
		Texture3D,
		STRUCTURED,
		Max
	};

	//テクスチャの種類
	enum class TextureType {
		RenderTarget=0,
		DepthBuffer,
		UnorderAccess,
		Max
	};

	//depth and stencil
	struct DepthStencil
	{
		float depth;
		uint8_t stencil;
	};

	//テクスチャの情報
	struct TextureDesc {
		TextureDimension dimension;
		uint32_t width;
		uint32_t height;
		uint32_t depthOrArraySize;
		uint32_t miplevels;
		DXGI_FORMAT format;
		uint32_t sampleCount;
		uint32_t sampleQuality = 0;
		TextureType type;
		D3D12_RESOURCE_STATES currentState;
		union 
		{
			float color[4];
			DepthStencil depthStencil;
		};
	};
	
	class Texture:public IResource
	{
	public:
		Texture();
		~Texture() {
			Term();
		}
		DECLMOVECOPY(Texture);

		bool Init(D3DDevice* dev, TextureDesc const& desc);
		bool InitStructuredBuffer(D3DDevice* dev, size_t const buffSize);
		bool InitFromDXImage(D3DDevice* dev,
			CommandList* cmdList,
			DirectX::ScratchImage const* image);
		bool InitFromPNG(
			D3DDevice* _dev,
			CommandList* _cmdList,
			void const* pPng,
			size_t size,
			uint32_t mipL
		);
		bool InitFromSwapChain(D3DDevice* _dev, SwapChain* _sc, uint32_t _bufferIdx);
		void Term(){}

		auto& GetTextureDesc()const {
			return m_Desc;
		}
		ID3D12Resource* GetRes()const{
			return m_pResource.Get();
		}
		virtual ID3D12Resource* GetResource()const override {
			return m_pResource.Get();
		}

		bool UploadAndCopyStructuredResource(
			D3DDevice* dev,
			CommandList* cmdList,
			uint8_t const* data,
			size_t const size
		);


	private:
		bool UploadAndCopyImage(
			D3DDevice* dev,
			CommandList* cmdList,
			DirectX::ScratchImage const* image,
			ID3D12Resource** ppResource
		);

		unqPtr<DirectX::ScratchImage> LoadPNG(void const* pPng, size_t size, uint32_t mipL);
		TextureDesc m_Desc;
		ComPtr<ID3D12Resource> m_pResource;


	};
}


