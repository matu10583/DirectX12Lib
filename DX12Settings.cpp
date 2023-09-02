#include "D3D12FrameWork/DX12Settings.h"

namespace D3D12FrameWork{
	const std::filesystem::path DX12Settings::SHADER_PATH = L"./shader/";
	const std::filesystem::path DX12Settings::SHADER_PDB_PATH = DX12Settings::SHADER_PATH / L"pdb/";
	const std::string DX12Settings::VS_ENTRY = "VSmain";
	const std::string DX12Settings::PS_ENTRY = "PSMain";
	//égÇÌÇ»Ç¢Ç∆évÇ§ÅB
	const std::string DX12Settings::VS_VERSION = "vs_5_0";
	const std::string DX12Settings::PS_VERSION = "ps_5_0";
	const DX12Settings::ShaderVersion DX12Settings::SHADER_VERSION = DX12Settings::VERSION_6_5;

	

	//bool 
	//	DX12Settings::CollectTimestamp(
	//		std::wstring _root, std::map<std::wstring, std::chrono::system_clock::duration>* _timeList,
	//		bool isRecursive) {
	//	std::filesystem::path root(_root);
	//	if (std::filesystem::is_directory(root)) return false;
	//	for (std::filesystem::directory_entry const& it :
	//		std::filesystem::recursive_directory_iterator(root)) {
	//		if (it.is_regular_file()) {
	//			(*_timeList)[it.path().wstring()] =
	//				std::filesystem::last_write_time(it.path()).time_since_epoch();
	//		}
	//	}
	//	return true;
	//}
}
