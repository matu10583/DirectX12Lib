#include "D3D12FrameWork/GlobalDescriptorHeap.h"
#include "D3D12FrameWork/DescriptorChankAllocator.h"
#include "D3D12FrameWork/RootParameterBuffer.h"
#include "D3D12FrameWork/RPBufferView.h"

namespace D3D12FrameWork{

	bool
		GlobalDescriptorHeap::Init(
			D3DDevice* _pDev,
			GlobalDescriptorHeapDesc const& _desc) {
		m_descChankAllocators.clear();
		if (!InitDescChankAllocator(_pDev,
			_desc.CBV_SRV_UAV, RPBufferType::CBV_SRV_UAV)) {
			assert(false);
			return false;
		}

		if (!InitDescChankAllocator(_pDev,
			_desc.SMP, RPBufferType::SMP)) {
			assert(false);
			return false;
		}	
		//if (!InitDescChankAllocator(_pDev,
		//	_desc.DSV, HeapType::DSV)) {
		//	assert(false);
		//	return false;
		//}
		//if (!InitDescChankAllocator(_pDev,
		//	_desc.RTV, HeapType::RTV)) {
		//	assert(false);
		//	return false;
		//}
		return true;
	}

	bool GlobalDescriptorHeap::InitDescChankAllocator(D3DDevice* _pDev,
		DescriptorAllocatorDesc const& _desc,
		RPBufferType _type) {
		m_descChankAllocators[_type].reset(
			new DescriptorChankAllocator()
		);
		auto heapdesc = D3D12_DESCRIPTOR_HEAP_DESC();
		heapdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		if (_type == RPBufferType::CBV_SRV_UAV ||
			_type == RPBufferType::SMP) {
			heapdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		}
		heapdesc.NodeMask = 0;
		heapdesc.NumDescriptors = _desc.NumDescriptor;
		switch (_type)
		{
		case D3D12FrameWork::RPBufferType::CBV_SRV_UAV:
			heapdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			break;
		case D3D12FrameWork::RPBufferType::RTV:
			heapdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			break;
		case D3D12FrameWork::RPBufferType::DSV:
			heapdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			break;
		case D3D12FrameWork::RPBufferType::SMP:
			heapdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
			break;
		default:
			assert(false);
			break;
		}
		return m_descChankAllocators[_type]->Init(
			_pDev,
			heapdesc
		);
	}

	bool 
		GlobalDescriptorHeap::RegisterRPChankInfo(
			uint32_t _rpIdx, RPBufferType _type, size_t _numDescriptors
		) {
		if (m_rpChankInfo.size() <= _rpIdx) {
			m_rpChankInfo.resize(_rpIdx+1);
		}
		auto& descChkInfo = m_rpChankInfo[_rpIdx].DescChankInfo;
		if (descChkInfo.pAllocator != nullptr) {
			descChkInfo.Free();
		}
		if (m_descChankAllocators[_type].get() == nullptr) {
			assert(false);
			return false;
		}
		descChkInfo = m_descChankAllocators[_type]->Allocate(static_cast<uint32_t>(_numDescriptors));
	//バージョンはあってもなくてもチャンと動くようにする。要するにnull状態もバージョンと考えて設計
		return true;
	}

	//heapを作り直しているならfalseを返す。heapの場所自体が変わっていないならcommandlistに設定するアドレス自体は変わらないはず
	bool 
		GlobalDescriptorHeap::CopyToHeapChank(class D3DDevice* _pDev,
			RootParameterCopyDesc const& _handleSet, uint32_t _rpIdx, RPBufferType const _type) {
		bool isLayoutStable = true;
		size_t descNum = 0;
		for (auto const& range : _handleSet.Ranges) {
			descNum += range.NumDescriptors;
		}
		auto const& descChkInfo = m_rpChankInfo[_rpIdx].DescChankInfo;
		if (m_rpChankInfo.size() <= _rpIdx ||
			descChkInfo.pAllocator == nullptr) {
			assert(false && "必要なrpChankInfoが作られていない．");
			RegisterRPChankInfo(_rpIdx, _type, descNum);
		}

		if (descChkInfo.NumDescriptors != descNum) {//ヒープを作り直す．
			isLayoutStable = false;
			auto htype = descChkInfo.pAllocator->Type();
			for (auto& ci : m_rpChankInfo) {
				if (ci.DescChankInfo.pAllocator != nullptr &&
					htype == ci.DescChankInfo.pAllocator->Type()) {
					ci.DescChankInfo.Free();
				}
			}
			auto desc = DescriptorAllocatorDesc{
				.NumDescriptor = descChkInfo.pAllocator->Size()
			};
			InitDescChankAllocator(_pDev, desc, ToHeapType(htype));
			RegisterRPChankInfo(_rpIdx, _type, descNum);

		}

		//auto& versions = m_rpChankInfo[_rpIdx].RangeVersions;
		//auto diff = _handleSet.Ranges.size() - versions.size();
		//if (diff>0) {
		//	versions.reserve(_handleSet.Ranges.size());
		//	for (auto numDiff = 0u; numDiff < diff; numDiff++) {
		//		versions.emplace_back(0);
		//	}
		//}


		auto heapStart = descChkInfo.CpuHandle;
		for (auto i=0u;i<_handleSet.Ranges.size();i++) {
			auto const& handles = _handleSet.Ranges[i];
			auto copySize = handles.NumDescriptors * descChkInfo.pAllocator->IncrementSize();
			//if (handles.Version == versions[i]) {//変更の必要なし。
			//	heapStart.ptr += copySize;
			//	continue;
			//}
			//ここでのサイズは個数を意味するらしい
			auto sizesInRange = std::vector<UINT>(handles.HandlesInRange.size(),1);
			_pDev->GetDev()->CopyDescriptors(
				1, //copy先は連続した一つのヒープ
				&heapStart, 
				&handles.NumDescriptors,//コピー先のヒープにはコピー元のハンドルの個数分コピーする．
				handles.NumDescriptors,//copy元はばらばらの個数分のヒープ
				handles.HandlesInRange.data(),
				sizesInRange.data(),//それぞれのヒープには一つずつハンドルがある，．
				descChkInfo.pAllocator->Type()
			);
			heapStart.ptr += copySize;
			//rangeのバージョン更新
			//versions[i] = handles.Version;
		}
		return isLayoutStable;
	}

	DescriptorChankInfo const& 
		GlobalDescriptorHeap::GetRPDescriptorHeapChank(uint32_t _rpIdx)const {
		if (m_rpChankInfo.size() <= _rpIdx ||
			m_rpChankInfo[_rpIdx].DescChankInfo.pAllocator == nullptr) {
			throw new std::runtime_error("rpが登録されていない");
		}
		return m_rpChankInfo[_rpIdx].DescChankInfo;
	}

	RPBufferType 
		GlobalDescriptorHeap::ToHeapType(D3D12_DESCRIPTOR_HEAP_TYPE const _type) {
		switch (_type)
		{
		case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
			return RPBufferType::CBV_SRV_UAV; break;
		case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
			return RPBufferType::RTV; break;
		case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
			return RPBufferType::DSV; break;
		case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
			return RPBufferType::SMP; break;
		default:
			assert(false);
			return RPBufferType::CBV_SRV_UAV;
			break;
		}
	}
}
