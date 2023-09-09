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
	//�o�[�W�����͂����Ă��Ȃ��Ă��`�����Ɠ����悤�ɂ���B�v�����null��Ԃ��o�[�W�����ƍl���Đ݌v
		return true;
	}

	//heap����蒼���Ă���Ȃ�false��Ԃ��Bheap�̏ꏊ���̂��ς���Ă��Ȃ��Ȃ�commandlist�ɐݒ肷��A�h���X���͕̂ς��Ȃ��͂�
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
			assert(false && "�K�v��rpChankInfo������Ă��Ȃ��D");
			RegisterRPChankInfo(_rpIdx, _type, descNum);
		}

		if (descChkInfo.NumDescriptors != descNum) {//�q�[�v����蒼���D
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
			//if (handles.Version == versions[i]) {//�ύX�̕K�v�Ȃ��B
			//	heapStart.ptr += copySize;
			//	continue;
			//}
			//�����ł̃T�C�Y�͌����Ӗ�����炵��
			auto sizesInRange = std::vector<UINT>(handles.HandlesInRange.size(),1);
			_pDev->GetDev()->CopyDescriptors(
				1, //copy��͘A��������̃q�[�v
				&heapStart, 
				&handles.NumDescriptors,//�R�s�[��̃q�[�v�ɂ̓R�s�[���̃n���h���̌����R�s�[����D
				handles.NumDescriptors,//copy���͂΂�΂�̌����̃q�[�v
				handles.HandlesInRange.data(),
				sizesInRange.data(),//���ꂼ��̃q�[�v�ɂ͈���n���h��������C�D
				descChkInfo.pAllocator->Type()
			);
			heapStart.ptr += copySize;
			//range�̃o�[�W�����X�V
			//versions[i] = handles.Version;
		}
		return isLayoutStable;
	}

	DescriptorChankInfo const& 
		GlobalDescriptorHeap::GetRPDescriptorHeapChank(uint32_t _rpIdx)const {
		if (m_rpChankInfo.size() <= _rpIdx ||
			m_rpChankInfo[_rpIdx].DescChankInfo.pAllocator == nullptr) {
			throw new std::runtime_error("rp���o�^����Ă��Ȃ�");
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
