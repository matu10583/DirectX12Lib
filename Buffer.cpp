#include "D3D12FrameWork/Buffer.h"
#include "D3D12FrameWork/D3DDevice.h"
#include "D3D12FrameWork/CommandList.h"

namespace D3D12FrameWork{
	Buffer::Buffer()
		:m_pResource(nullptr)
		,m_pBuffer(nullptr)
		,m_buffer_type(BufferType::MAX)
		,m_buffSize(0) {}

	Buffer::~Buffer() {
		Term();
	}

	bool Buffer::Init(
		D3DDevice* _pDev,
		uint32_t _width,
		BufferType _type
	) {
		switch (_type)
		{
		case D3D12FrameWork::BUFFER_TYPE_UPLOAD:
			return InitAsUploadBuffer(_pDev, _width);
			break;
		case D3D12FrameWork::BUFFER_TYPE_DEFAULT:
			return InitAsDefaultBuffer(_pDev, _width);
			break;
		case D3D12FrameWork::MAX:
		default:
			assert(false);
			break;
		}
		return true;
		
	}

	bool Buffer::InitAsUploadBuffer(
		D3DDevice* _pDev,
		uint32_t _width
	) {
		m_buffSize = (_width + 0xff) & ~0xff;
		D3D12_HEAP_PROPERTIES heapProp = {  };
		heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
		m_buffer_type = BufferType::BUFFER_TYPE_UPLOAD;

		heapProp.CreationNodeMask = 0;
		heapProp.VisibleNodeMask = 0;
		heapProp.MemoryPoolPreference =
			D3D12_MEMORY_POOL_UNKNOWN;
		
		D3D12_RESOURCE_DESC resDesc = {};
		resDesc.Dimension =
			D3D12_RESOURCE_DIMENSION_BUFFER;
		resDesc.Format =
			DXGI_FORMAT_UNKNOWN;
		resDesc.MipLevels = 1;
		resDesc.Alignment = 0;
		resDesc.DepthOrArraySize = 1;
		resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		resDesc.Height = 1;
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resDesc.SampleDesc.Count = 1;
		resDesc.SampleDesc.Quality = 0;
		resDesc.Width = m_buffSize;

		auto hr = _pDev->GetDev()->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_pResource.ReleaseAndGetAddressOf())
		);

		if (FAILED(hr)) {
			assert(false);
			return false;
		}
		m_buffer_type = BufferType::BUFFER_TYPE_UPLOAD;
		return true;
	}
	bool Buffer::InitAsDefaultBuffer(
		D3DDevice* _pDev,
		uint32_t _size_in_bytes,
		CommandList* _pCmdList,
		uint8_t const* _pResource
	) {
		m_buffSize = (_size_in_bytes + 0xff) & ~0xff;
		D3D12_HEAP_PROPERTIES heapProp = {  };
		heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
		m_buffer_type = BufferType::BUFFER_TYPE_DEFAULT;

		heapProp.CreationNodeMask = 0;
		heapProp.VisibleNodeMask = 0;
		heapProp.MemoryPoolPreference =
			D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC resDesc = {};
		resDesc.Dimension =
			D3D12_RESOURCE_DIMENSION_BUFFER;
		resDesc.Format =
			DXGI_FORMAT_UNKNOWN;
		resDesc.MipLevels = 1;
		resDesc.Alignment = 0;
		resDesc.DepthOrArraySize = 1;
		resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		resDesc.Height = 1;
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resDesc.SampleDesc.Count = 1;
		resDesc.SampleDesc.Quality = 0;
		resDesc.Width = m_buffSize;

		auto hr = _pDev->GetDev()->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(m_pResource.ReleaseAndGetAddressOf())
		);

		if (FAILED(hr)) {
			assert(false);
			return false;
		}
		m_buffer_type = BufferType::BUFFER_TYPE_DEFAULT;
		if (_pResource == nullptr) {
			return false;//コピーは行わない
		}

		if (!CopyToResource(
			_pDev, _pCmdList,
			_pResource,
			_size_in_bytes
		)) {
			assert(false);
			return false;
		}


		return true;
	}

	bool
		Buffer::CopyToResource(
			D3DDevice* dev,
			CommandList* cmdList,
			const uint8_t* _pData,
			size_t _size_in_bytes,
			size_t _offset
		) {
		if (m_buffer_type != BufferType::BUFFER_TYPE_DEFAULT) return false;

		//アップロード用のオブジェクト
		D3D12_HEAP_PROPERTIES heapProp = {};
		heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProp.CreationNodeMask = 0;
		heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapProp.VisibleNodeMask = 0;

		D3D12_RESOURCE_DESC resDesc = {};

		resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		resDesc.Height = 1;
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resDesc.MipLevels = 1;
		resDesc.SampleDesc.Count = 1;
		resDesc.SampleDesc.Quality = 0;
		resDesc.Width = _size_in_bytes;
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

		memcpy(pMappedSrc, _pData, _size_in_bytes);
		
		pUploadSrc->Unmap(0, nullptr);

		//コピー
		
		cmdList->GetList()->CopyBufferRegion(m_pResource.Get(),
			_offset,
			pUploadSrc, 0,
			_size_in_bytes);

		dev->PendingRelease<ID3D12Resource>(pUploadSrc);
		return true;
	}

	bool Buffer::Map() {
		if (!m_pResource.Get()) {
			assert(false);
			return false;
		}
		if (m_buffer_type != BufferType::BUFFER_TYPE_UPLOAD) return false;

		auto hr = m_pResource->Map(
			0, nullptr,
			&m_pBuffer
		);
		if (FAILED(hr)) {
			assert(false);
			return false;
		}
		return true;
	}



	void Buffer::UnMap() {
		m_pResource->Unmap(0, nullptr);
		m_pBuffer = nullptr;
	}

	void Buffer::Term() {
		m_pBuffer = nullptr;
	}
}
