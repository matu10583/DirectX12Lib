#include "D3D12FrameWork/RPBufferView.h"
#include "D3D12FrameWork/RootParameterBuffer.h"
#include "D3D12FrameWork/ConstantBufferSet.h"
#include "D3D12FrameWork/TextureView.h"


namespace D3D12FrameWork{
	RootParameterCopyDesc 
		RPBufferView::GetCopyDesc()const {
		std::vector<RangeCopyDesc> ret;
		auto const& ranges = m_rpBuffer.get().GetRanges();
		std::transform(
			ranges.begin(), ranges.end(),
			std::back_inserter(ret),
			[](IDescriptorRangeSet const* _range) {
				return _range->GetCopyDesc();
			}
		);
		return RootParameterCopyDesc{
			.Ranges = std::move(ret)
		};
	}

	RPBufferType const
		RPBufferView::GetType()const {
		return m_rpBuffer.get().GetType();
	}

	size_t const
		RPBufferView::GetNumViews()const {
		size_t ret = 0;
		for (auto const range : m_rpBuffer.get().GetRanges()) {
			ret += range->NumViews();
		}
		return ret;
	}

	RenderTargetView const* 
		RPBufferView::TextureRTV(std::string_view _regName) {
		auto tex_set = m_rpBuffer.get().FindTextureFromRegisterName(_regName);
		auto rtv = tex_set->GetRenderTargetView(_regName);
		return rtv;
	}
}
