#pragma once
#include <filesystem>
#include <fstream>
#include "D3D12FrameWork/ComPtr.h"
#include "D3D12FrameWork/Common.h"
namespace D3D12FrameWork {
	class File
	{
	public:
		File()
			:m_pData(nullptr)
			,m_size(0)
		{}
		~File() {
			Term();
		}

		bool ReadFile(std::filesystem::path const& _filePath, bool isBin = true);
		void Term();

		GETTERPTR(uint8_t, Data, m_pData.get());
		auto GetSize() const{
			return m_size;
		}

		static bool WriteFile(std::filesystem::path const& _filePath, size_t _size,
			uint8_t* _pData, bool isBin=true, bool isAdd=false);

	private:
		uint64_t m_size;
		unqPtr<uint8_t[]> m_pData;
	};
}



