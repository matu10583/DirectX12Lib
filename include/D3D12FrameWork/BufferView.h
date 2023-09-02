#pragma once
#include "D3D12FrameWork/TextureView.h"
#include "D3D12FrameWork/DescriptorAllocator.h"
namespace D3D12FrameWork {
	class Buffer;
	class ConstantBufferView:public IResourceView
	{
	public:
		ConstantBufferView();
		~ConstantBufferView();
		DECLMOVECOPY(ConstantBufferView);
		bool Init(
			D3DDevice const* _pDev,
			Buffer const* _pBuff,
			const size_t _offset = 0,
			const size_t _size = 0
		);
		virtual DescriptorInfo const&
			GetHeapInfo()const override {
			return m_descInfo;
		}

		static ConstantBufferView const& NullView() {
			return *m_pNullView;
		}
		static bool CreateNullView(D3DDevice const* _pDev) {
			if (_pDev == nullptr || m_pNullView != nullptr) return false;
			m_pNullView = new ConstantBufferView();
			m_pNullView->m_descInfo = _pDev->GetCbvSrvUavHeap()->Allocate();
			_pDev->GetDev()->CreateConstantBufferView(
				nullptr, m_pNullView->m_descInfo.m_CpuHandle
			);
			return true;
		}
		static void DestroyNullView() {
			delete m_pNullView;
			m_pNullView = nullptr;
		}


	private:
		DescriptorInfo m_descInfo;
		void Term() { m_descInfo.Free(); }
		static ConstantBufferView* m_pNullView;

	};

}


