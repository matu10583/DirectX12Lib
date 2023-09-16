#pragma once
#include "D3D12FrameWork/Common.h"
namespace D3D12FrameWork {
	class RootParameterBuffer;
	class D3DDevice;
	struct RootParameterCopyDesc
	{
		std::vector<
			struct RangeCopyDesc
		> Ranges;
	};
	class RPBufferView
	{
	public:
		RPBufferView() = delete;
		RPBufferView(RootParameterBuffer& _rpBuffer)
			:m_rpBuffer(_rpBuffer){}
		~RPBufferView() = default;
		DECLMOVECOPY(RPBufferView);

		RootParameterCopyDesc GetCopyDesc()const;
		enum RPBufferType const GetType()const;
		size_t const GetNumViews()const;
		RootParameterBuffer* GetRPBuff()const {
			return &m_rpBuffer.get();
		}

		class RenderTargetView const* TextureRTV(std::string_view _regName);



	private:
		std::reference_wrapper<RootParameterBuffer> const m_rpBuffer;
	};
}


