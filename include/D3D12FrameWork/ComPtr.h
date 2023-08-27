#pragma once

#include <wrl/client.h>
#include <memory>
#include <d3d12.h>
namespace D3D12FrameWork {
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	template<typename T>
	using shrPtr = std::shared_ptr<T>;
	template<typename T>
	using unqPtr = std::unique_ptr<T>;
}

