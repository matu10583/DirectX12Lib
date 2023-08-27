#pragma once
#include "D3D12FrameWork/TextureView.h"
#include "D3D12FrameWork/DescriptorAllocator.h"
#include "D3D12FrameWork/D3DDevice.h"

namespace D3D12FrameWork {
	class SamplerView:public IResourceView
	{
	public:
		SamplerView() = default;
		~SamplerView() {
			m_descInfo.Free();
		}
		DECLMOVECOPY(SamplerView);
		bool Init(
			D3DDevice* _pDev,
			D3D12_SAMPLER_DESC const* _pSmpDesc=nullptr) {
			auto allocator = _pDev->GetSmpHeaps();
			m_descInfo = allocator->Allocate();
			if (_pSmpDesc == nullptr) return true;
			return UpdateSampler(_pDev, *_pSmpDesc);
		}
		bool UpdateSampler(
			D3DDevice* _pDev,
			D3D12_SAMPLER_DESC const& _smpDesc
		) {
			_pDev->GetDev()->CreateSampler(&_smpDesc, m_descInfo.m_CpuHandle);
			return true;
		}

		virtual DescriptorInfo const&
			GetHeapInfo()const override {
			return m_descInfo;
		}

		static auto const& NullView() {
			return *m_pNullView;
		}
		static bool CreateNullView(D3DDevice const* _pDev) {
			if (_pDev == nullptr || m_pNullView != nullptr) return false;
			m_pNullView = new SamplerView();
			m_pNullView->m_descInfo = _pDev->GetSmpHeaps()->Allocate();
			auto smpDesc = D3D12_SAMPLER_DESC{
				.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT,
				.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
				.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
				.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
				.MipLODBias = 0,
				.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
				.MinLOD = 0.0f,
				.MaxLOD = D3D12_FLOAT32_MAX
			};

			_pDev->GetDev()->CreateSampler(
				&smpDesc, m_pNullView->m_descInfo.m_CpuHandle
			);
			return true;
		}
		static void DestroyNullView() {
			delete m_pNullView;
			m_pNullView = nullptr;
		}


	private:
		DescriptorInfo m_descInfo;
		static SamplerView* m_pNullView;
	};
}



