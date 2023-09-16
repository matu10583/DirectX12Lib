#include "D3D12FrameWork/TextureView.h"
#include "D3D12FrameWork/Texture.h"
#include "D3D12FrameWork/D3DDevice.h"
#include <cassert>

namespace D3D12FrameWork {
	RenderTargetView::RenderTargetView()
		:m_descInfo() 
	{}

	RenderTargetView::~RenderTargetView() {
		Term();
	}

	bool RenderTargetView::Init(
		D3DDevice* _pDev,
		Texture* _pTexture,
		uint32_t _mipSlice
	) {
		if (!_pDev) {
			assert(false);
			return false;
		}
		m_descInfo = _pDev->GetRTVHeaps()->Allocate();
		if (_pTexture != nullptr) {
			return UpdateView(
				_pDev, _pTexture, _mipSlice
			);
		}
		return true;
	}

	bool RenderTargetView::UpdateView(
		D3DDevice* _pDev,
		Texture* _pTexture,
		uint32_t _mipSlice) {
		auto texDesc = _pTexture->GetTextureDesc();
		D3D12_RENDER_TARGET_VIEW_DESC vdesc = {};
		vdesc.Format = texDesc.format;
		
		if (texDesc.dimension == TextureDimension::Texture1D) {
			vdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
			vdesc.Texture1D.MipSlice = _mipSlice;
		}
		else if (texDesc.dimension == TextureDimension::Texture2D) {
			vdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			vdesc.Texture2D.MipSlice = _mipSlice;
			vdesc.Texture2D.PlaneSlice = 0;
		}
		else if (texDesc.dimension == TextureDimension::Texture3D) {
			//vdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
			//vdesc.Texture3D.MipSlice = _mipSlice;
			//vdesc.Texture3D.FirstWSlice=
			//	vdesc.Texture3D.WSize=//‚æ‚­‚í‚©‚ç‚È‚¢‚Ì‚ÅŒã‚ÅŽÀ‘•
			assert(false);
		}

		_pDev->GetDev()->CreateRenderTargetView(
			_pTexture->GetRes(),
			&vdesc,
			m_descInfo.m_CpuHandle
		);

		return true;
	}

	void RenderTargetView::Term() {
		m_descInfo.Free();
	}

	//----------------------------------------

	ShaderResourceView* ShaderResourceView::m_pNullView = nullptr;

	ShaderResourceView::ShaderResourceView()
		:m_descInfo()
	{}
	ShaderResourceView::~ShaderResourceView() {
		Term();
	}
	bool ShaderResourceView::Init(
		D3DDevice* _pDev,
		Texture* _pTexture,
		uint32_t _mostDetailedMipL
	) {
		if (!_pDev) {
			assert(false);
			return false;
		}
		m_descInfo = _pDev->GetCbvSrvUavHeap()->Allocate();
		if (_pTexture != nullptr) {
			return UpdateView(
				_pDev, _pTexture, _mostDetailedMipL
			);
		}


		return true;
	}

	bool ShaderResourceView::UpdateView(
		D3DDevice* _pDev,
		Texture* _pTexture,
		UINT _mostDetailedMipL) {
		auto texDesc = _pTexture->GetTextureDesc();
		D3D12_SHADER_RESOURCE_VIEW_DESC vdesc = {};
		vdesc.Format = texDesc.format;
		if (texDesc.dimension == TextureDimension::Texture1D) {
			vdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
			vdesc.Texture1D.MipLevels = texDesc.miplevels;
			vdesc.Texture1D.MostDetailedMip = _mostDetailedMipL;
			vdesc.Texture1D.ResourceMinLODClamp = 0;
		}
		else if (texDesc.dimension == TextureDimension::Texture2D) {
			vdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			vdesc.Texture2D.MipLevels= texDesc.miplevels;
			vdesc.Texture2D.MostDetailedMip = _mostDetailedMipL;
			vdesc.Texture2D.ResourceMinLODClamp = 0;
			vdesc.Texture2D.PlaneSlice = 0;
		}
		else if (texDesc.dimension == TextureDimension::Texture3D) {
			//vdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			//vdesc.Texture3D.MipSlice = _mipSlice;
			//vdesc.Texture3D.FirstWSlice=
			//	vdesc.Texture3D.WSize=//‚æ‚­‚í‚©‚ç‚È‚¢‚Ì‚ÅŒã‚ÅŽÀ‘•
			assert(false);
		}

		vdesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		_pDev->GetDev()->CreateShaderResourceView(
			_pTexture->GetRes(),
			&vdesc,
			m_descInfo.m_CpuHandle
		);

		return true;
	}

	bool 
		ShaderResourceView::UpdateStructuredBufferView(
		D3DDevice* _pDev,
		Texture* _pTexture,
		size_t _sizeElement,
		UINT _numElements,
		UINT _startIndex
	) {
		auto texDesc = _pTexture->GetTextureDesc();
		D3D12_SHADER_RESOURCE_VIEW_DESC vdesc = {};
		vdesc.Format = texDesc.format;
		assert(texDesc.dimension == TextureDimension::STRUCTURED);
		vdesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		vdesc.Buffer.FirstElement = _startIndex;
		vdesc.Buffer.NumElements = _numElements;
		vdesc.Buffer.StructureByteStride = _sizeElement;
		vdesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		_pDev->GetDev()->CreateShaderResourceView(
			_pTexture->GetRes(),
			&vdesc,
			m_descInfo.m_CpuHandle
		);
		return true;
	}

	void ShaderResourceView::Term() {
		m_descInfo.Free();
	}
}
