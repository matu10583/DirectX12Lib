#pragma once
#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/ComPtr.h"
#include "D3D12FrameWork/ShaderBindInfo.h"
#include "D3D12FrameWork/BufferView.h"
#include "D3D12FrameWork/Buffer.h"
#include "D3D12FrameWork/RootSignature.h"
#include <unordered_map>

namespace D3D12FrameWork {//DescTblRangeが対応する．
	struct RangeCopyDesc
	{
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> HandlesInRange;
		//bool doNeedupdate;//これがfalseの場合vectorは存在していないです．
		uint32_t NumDescriptors;
		//コマンドリストが持つ値と比較して異なっていたらコピーする。
		//unsigned int Version;
	};

	struct ConstantStructInfo
	{
		size_t const Offset;
		std::unordered_map<std::string, ShaderRegisterDescs::ConstantVariableDesc> const*
			MemberDesc;
		std::vector<ConstantBufferView> CBVs;
		ConstantStructInfo()
			:Offset(0)
			,MemberDesc(nullptr),
			CBVs()
		{}
		ConstantStructInfo(size_t const ofst, 
			std::unordered_map<std::string, ShaderRegisterDescs::ConstantVariableDesc> const* const pMemDesc)
			:Offset(ofst)
			, MemberDesc(pMemDesc)
			,CBVs()
		{}
	};

	class IDescriptorRangeSet {
	public:
		IDescriptorRangeSet() = default;
		virtual ~IDescriptorRangeSet() = default;
		DECLMOVECOPY(IDescriptorRangeSet);
		virtual std::vector<std::reference_wrapper<IResourceView const>> const GetViews()const = 0;
		virtual RangeCopyDesc GetCopyDesc()const = 0;
		//virtual unsigned int const DescriptorHeapVersion()const = 0;
		virtual size_t NumViews() const = 0;
	};
	class ConstantBufferSet:public IDescriptorRangeSet
	{
	public:
		ConstantBufferSet();
		~ConstantBufferSet();
		DECLMOVECOPY(ConstantBufferSet);

		bool InitFromRegisterDescWithRange(
			std::unordered_map<std::string, ShaderRegisterDescs::ConstantBindDesc>const&,
			std::vector<ShaderRegisterDescs::RegisterDesc> const&,
			UINT _rangeStart,
			UINT _rangeNum,
			BufferType _type,
			class D3DDevice* _pDev,
			uint32_t _bufferCount);
		bool CopyToMappedPtr(
			std::string_view _regName, uint8_t const* _pData, size_t _size
			, size_t _offset_in_struct = 0,bool copyToBackBuffers=false
		);

		bool CopyToResource(D3DDevice* _pdev, CommandList* _pcmdList,
			std::string_view _regName,uint8_t const* _pData, size_t _size
			, size_t _offset_in_struct=0, bool copyToBackBuffers = false);

		bool HasStruct(std::string_view _struct_name)const {
			return m_structDescs.count(std::string(_struct_name.data()));
		}
		bool HasVariable(std::string_view _var_name, size_t* const _offset=nullptr)const {
			for (auto const& st : m_structDescs) {
				if (st.second.MemberDesc->count(_var_name.data())) {
					if (_offset != nullptr) *_offset =
						st.second.MemberDesc->at(_var_name.data()).StartOffset;
					return true;
				}
			}
			return false;
		}

		std::vector<std::reference_wrapper<IResourceView const>> const GetViews()const override {
			std::vector<std::reference_wrapper<IResourceView const>> ret;
			std::transform(m_orderedCBInfo.begin(), m_orderedCBInfo.end(),
				std::back_inserter(ret),
				[this](ConstantStructInfo const* css) {
					return (css != nullptr) ? std::cref(css->CBVs[GetNowFrameIndex()])
						: std::cref(ConstantBufferView::NullView());
				});
			return ret;
		}

		RangeCopyDesc GetCopyDesc()const override {
			std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> handles;
			std::transform(m_orderedCBInfo.begin(), m_orderedCBInfo.end(),
				std::back_inserter(handles),
				[this](ConstantStructInfo const* css) {
					return (css != nullptr) ? css->CBVs[GetNowFrameIndex()].GetHeapInfo().m_CpuHandle
						: ConstantBufferView::NullView().GetHeapInfo().m_CpuHandle;
				});
			return RangeCopyDesc{
				.HandlesInRange = std::move(handles),
				.NumDescriptors = (UINT)m_orderedCBInfo.size(),
				//.Version=m_descriptorVersion
			};
		}
		void Present() {
			if (m_needPresent == false) return;
			m_nowFrameIndex =
				(m_nowFrameIndex < m_pBuffers.size() - 1) ? m_nowFrameIndex + 1 : 0;
			//UpdateDescriptorHeap();//ビューが変わるので更新が必要
			m_needPresent = false;
		}
		size_t NumViews() const override { return m_orderedCBInfo.size(); }
		//void UpdateDescriptorHeap() {
		//	m_descriptorVersion = (m_descriptorVersion == UINT_MAX) ?
		//		0 : m_descriptorVersion + 1;
		//}
		//unsigned int const DescriptorHeapVersion()const override { return m_descriptorVersion; }
		void AfterDraw() {
			m_needPresent = true;
		}

	private:
		std::vector<unqPtr<Buffer>> m_pBuffers;
		std::unordered_map<std::string, ConstantStructInfo> m_structDescs;
		std::vector<ConstantStructInfo*> m_orderedCBInfo;
		//unsigned int m_descriptorVersion = 0;
		unsigned int m_nowFrameIndex = 0;
		bool m_needPresent = false;

		Buffer* GetNowBuffer()const {
			assert(m_nowFrameIndex < m_pBuffers.size());
			return m_pBuffers[m_nowFrameIndex].get();
		}
		unsigned int GetNowFrameIndex()const {
			return m_nowFrameIndex;
		}
		uint8_t* MappedPtr(std::string_view _regName, unsigned int _frameIdx);
	};
}


