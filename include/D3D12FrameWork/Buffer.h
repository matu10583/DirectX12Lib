#pragma once
#include "D3D12FrameWork/ComPtr.h"
#include "D3D12FrameWork/Common.h"

namespace D3D12FrameWork {
	class D3DDevice;
	class CommandList;
	enum BufferType {
		//gpu�̓L���b�V������f�[�^���擾���邪�C�f�[�^��invalid�Ȃ�cpu����pull����D�p�ɂɕύX���郊�\�[�X�͂�����
		BUFFER_TYPE_UPLOAD,
		//buffer��vram��ɔz�u�����D�s�ςȃf�[�^�͂�����
		BUFFER_TYPE_DEFAULT,
		MAX
	};



	class Buffer
	{
	public:
		Buffer();
		~Buffer();
		DECLMOVECOPY(Buffer);
		bool InitAsUploadBuffer(D3DDevice* _pDev,
			uint32_t _size);
		bool InitAsDefaultBuffer(D3DDevice* _pDev,
			uint32_t _size, CommandList* _pCmdList = nullptr,
			uint8_t const* _pResource=nullptr
			);
		bool Init(D3DDevice* _pDev,
			uint32_t _size,
			BufferType _type);


		bool Map();
		template<typename T>
		T* MappedPtr() {
			if (m_pBuffer == nullptr
				&& !Map()) {
				return nullptr;
			}
			if (m_buffer_type != BufferType::BUFFER_TYPE_UPLOAD) return nullptr;
			return reinterpret_cast<T*>(m_pBuffer);
		}
		void UnMap();

		bool CopyToResource(
			D3DDevice* _dev,
			CommandList* _cmdList,
			const uint8_t* _pData,
			size_t _size,
			size_t _offset=0);

		BufferType GetType()const {
			return m_buffer_type;
		}
		auto GetGPUVirtualAddress()const {
			return m_pResource->GetGPUVirtualAddress();
		}
		auto const GetSize()const {
			return m_buffSize;
		}
	private:
		void Term();
		

		ComPtr<ID3D12Resource> m_pResource;
		void* m_pBuffer;
		BufferType m_buffer_type;
		size_t m_buffSize;
		
	};

}

