#pragma once
#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/ConstantBufferSet.h"
#include "D3D12FrameWork/SamplerView.h"

namespace D3D12FrameWork {

	class SamplerSet:public IDescriptorRangeSet
	{
	public:
		SamplerSet()
			:m_doNeedUpdate(true){}
		bool InitFromRegisterDescWithRange(
			std::unordered_map<std::string, ShaderRegisterDescs::SamplerBindDesc>const&,
			std::vector<ShaderRegisterDescs::RegisterDesc> const&,
			UINT _rangeStart,
			UINT _rangeNum,
			class D3DDevice* _pDev);
		bool CreateSampler(D3DDevice* _pdev,
			std::string_view _regName,
			D3D12_SAMPLER_DESC const&);
		bool HasKey(std::string_view _name) const {
			return m_samplerMap.count(std::string(_name.data()));
		}
		std::vector<std::reference_wrapper<IResourceView const>> const& GetViews()const override {
			std::vector<std::reference_wrapper<IResourceView const>> ret;
			std::transform(m_samplerViewRef.begin(), m_samplerViewRef.end(),
				std::back_inserter(ret),
				[](SamplerView const* smp) {
					return (smp != nullptr) ? std::cref(*smp)
						: std::cref(SamplerView::NullView());
				});
			return ret;
		}
		
		RangeCopyDesc GetCopyDesc()const override {
			if (!m_doNeedUpdate) {
				return RangeCopyDesc{
					.NumDescriptors = (UINT)m_samplerViewRef.size()
				};
			}
			std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> handles;
			std::transform(m_samplerViewRef.begin(), m_samplerViewRef.end(),
				std::back_inserter(handles),
				[](SamplerView const* css) {
					return (css != nullptr) ? css->GetHeapInfo().m_CpuHandle
						: SamplerView::NullView().GetHeapInfo().m_CpuHandle;
				});
			return RangeCopyDesc{
				.HandlesInRange = std::move(handles),
				.NumDescriptors = (UINT)m_samplerViewRef.size(),
				//.Version=m_descriptorVersion
			};
		}
		size_t NumViews() const override { return m_samplerViewRef.size(); }
		//void UpdateDescriptorHeap() {
		//	m_descriptorVersion = (m_descriptorVersion == UINT_MAX) ?
		//		0 : m_descriptorVersion + 1;
		//}
		//unsigned int const DescriptorHeapVersion()const override { return m_descriptorVersion; }
	private:
		std::unordered_map<std::string, SamplerView> m_samplerMap;
		std::vector<SamplerView*> m_samplerViewRef;
		bool m_doNeedUpdate;
		//unsigned int m_descriptorVersion = 0;
	};
}


