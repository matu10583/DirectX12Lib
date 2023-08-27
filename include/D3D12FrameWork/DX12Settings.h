#pragma once
#include <stdint.h>
#include <string>
#include <map>
#include <filesystem>

namespace D3D12FrameWork {
	class DX12Settings
	{
	public:
		//static const uint32_t BUFFER_COUNT;
		static const std::wstring SHADER_PATH;
		static const std::string VS_ENTRY;
		static const std::string PS_ENTRY;
		static const std::string VS_VERSION;
		static const std::string PS_VERSION;

		static constexpr uint32_t BUFFER_COUNT=3;
	private:
		//static std::map<std::wstring, std::chrono::system_clock::duration> last_updated_time;
		//bool CollectTimestamp(std::wstring _root, 
		//	std::map<std::wstring, std::chrono::system_clock::duration>* _timeList,
		//bool isRecursive=true);
	};
}



