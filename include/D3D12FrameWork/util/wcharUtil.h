#pragma once
#include "D3D12FrameWork/Common.h"

namespace D3D12FrameWork{
	inline int GetWCharBuffSizeFromChar(std::string_view _str) {
		return MultiByteToWideChar(CP_ACP, 0, _str.data(),
			-1, NULL, 0);
	}

	inline bool StrToWStr(std::string_view _str, wchar_t* _outWC, int wbuffSize) {
		auto buffsize = GetWCharBuffSizeFromChar(_str);
		if (buffsize <= wbuffSize) {
			MultiByteToWideChar(CP_ACP, 0, _str.data(), -1, _outWC, buffsize);
			return true;
		}

		return false;
	}
}
