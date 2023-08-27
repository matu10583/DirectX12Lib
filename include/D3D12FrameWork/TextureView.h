#pragma once
#include "D3D12FrameWork/DescriptorAllocator.h"
#include "D3D12FrameWork/D3DDevice.h"
#include "D3D12FrameWork/Common.h"

//テクスチャ形式に関連するビュー
namespace D3D12FrameWork {
	class IResourceView {
	public:
		IResourceView() = default;
		virtual ~IResourceView() = default;
		DECLMOVECOPY(IResourceView);
		virtual DescriptorInfo const& GetHeapInfo()const=0;
	};
	class Texture;

	class RenderTargetView: public IResourceView
	{
	public:
		RenderTargetView();
		~RenderTargetView();
		DECLMOVECOPY(RenderTargetView);
		bool Init(
			D3DDevice* _pDev,
			Texture* _pTexture=nullptr,
			uint32_t _mipSlice=0
		);
		bool UpdateView(
			D3DDevice* _pDev,
			Texture* _pTexture,
			uint32_t _mipSlice = 0
		);

		virtual DescriptorInfo const&
			GetHeapInfo()const override {
			return m_descInfo;
		}
	private:
		DescriptorInfo m_descInfo;
		void Term();

	};

	class ShaderResourceView:public IResourceView
	{
	public:
		ShaderResourceView();
		~ShaderResourceView();
		DECLMOVECOPY(ShaderResourceView);
		bool Init(
			D3DDevice* _pDev,
			Texture* _pTexture = nullptr,
			uint32_t _mostDetailedMipL = 0
		);
		bool UpdateView(
			D3DDevice* _pDev,
			Texture* _pTexture,
			UINT _mostDetailedMipL = 0
		);

		virtual DescriptorInfo const&
			GetHeapInfo()const override {
			return m_descInfo;
		}

		static ShaderResourceView const& NullView() {
			return *m_pNullView;
		}
		static bool CreateNullView(D3DDevice const* _pDev) {
			if (_pDev == nullptr || m_pNullView!=nullptr) return false;
			m_pNullView = new ShaderResourceView();
			m_pNullView->m_descInfo = _pDev->GetCbvSrvUavHeap()->Allocate();
			//どうせアクセスしないなら適当でいいんじゃない？アクセスするならdescを合わせないと未定義らしい
			D3D12_SHADER_RESOURCE_VIEW_DESC desc{
				.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
				.Shader4ComponentMapping=D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING
			};
			_pDev->GetDev()->CreateShaderResourceView(
				nullptr, &desc,
				m_pNullView->m_descInfo.m_CpuHandle
			);
			return true;
		}
		static void DestroyNullView() {
			delete m_pNullView;
			m_pNullView = nullptr;
		}
	private:
		DescriptorInfo m_descInfo;
		void Term();
		static ShaderResourceView* m_pNullView;
	};
}


