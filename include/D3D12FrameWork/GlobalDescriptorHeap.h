#pragma once

#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/ComPtr.h"
#include "D3D12FrameWork/DescriptorChankAllocator.h"
#include <unordered_map>
namespace D3D12FrameWork {
	class D3DDevice;

	struct DescriptorAllocatorDesc
	{
		uint32_t NumDescriptor;
	};
	struct GlobalDescriptorHeapDesc
	{
		DescriptorAllocatorDesc CBV_SRV_UAV;
		DescriptorAllocatorDesc SMP;
		//DescriptorAllocatorDesc RTV;
		//DescriptorAllocatorDesc DSV;
	};
	enum RPBufferType;
	struct RootParameterChankInfo
	{
		DescriptorChankInfo DescChankInfo;
		//std::vector<unsigned int> RangeVersions;
	};
/// <summary>
/// �S�f�B�X�N���v�^�Ё[�Ԃ������ɃR�s�[���Ă����pso�ɐݒ肷��D���̂��߂̂��
/// rp�̃��C�A�E�g�͂قƂ�Ǖς��Ȃ��͂��D��Ƀ��C�A�E�g��ݒ肵�Čォ��R�s�[����̂��v�Z�ʓI�ɂ͊y
/// ���C�A�E�g�Ƃ����Ă�rp�̃T�C�Y���炢�����o�������Ȃ��Drp�̔ԍ���heapstart�Ƃ̑Ή����o���Ă����Ƃ悢�Ǝv���D
/// �R�s�[����Ƃ����l����Drp�Ɋ܂܂��view�̂��ꂼ��̏ꏊ�͂킩��͂��Dview�̃T�C�Y��desc��type��������Α��v
/// �R�s�[�֐��͈�̃^�C�v���Ɏ��s����K�v������D
/// ��������range���ƂɎ󂯎��Ȃ���΂Ȃ�Ȃ��D
/// rpBuffer����orderedRangeSet���擾����D���ꂼ���GetView�����邱�Ƃɂ����IresourceView�̔z��𓾂���D
/// �R�s�[���ǂ̒P�ʂōs�����D�Ƃ肠����range���Ƃ̃R�s�[�ł����񂶂�Ȃ����D
/// �ꉞ���W�X�^�P�ʂł̃R�s�[�̉ۂ��o���Ȃ��͂Ȃ�����rp�����̂Ƃ����؂�ɂȂ��Ă���D
/// �܂�rp�Ɉꏏ�Ɋ܂܂��p�����[�^�[�͈ꏏ�ɕς�����\���������p�����[�^�[�C
/// �����l�����rp�ł܂Ƃ߂ăR�s�[���Ă��������炢�����Crange���ƂɃR�s�[�֐�������̂�
/// range���ƂɌ��肵�������y�C�܂���q�̗��R���炻��ŏ\�����낤�Ƃ����Ƃ���ł��D
/// ���̂Ƃ��남���炭range��heap�͘A���Ȃ̂�simple���g���Ă��������ǂ��Ȃ炸�A���ɂȂ�킯�łȂ��̂łƂ肠�������ʂ̊֐����g���D
/// 
/// rp���ƂɃq�[�v�ɃR�s�[����֐����l����Drp�ɑΉ�����heapstart�ƃT�C�Y�͊o���Ă����D
/// ���O�ɃR�s�[����K�v������̂��̓t���O�Œ��ׂĂ����D
/// ���͂Ƃ��Ă�((cpuhandle��vector)��range���Ƃ�vector)�Ⴆ��cbv,srv�ł�cbv��handle�̃x�N�^�[�Csrv�̃n���h���̃x�N�^�[.range�̏��Ԓʂ�
/// ����range���ƂɃR�s�[����K�v������̂�
/// �T�C�Y�̃`�F�b�N�Dvector�̑�������T�C�Y���v�Z����D�T�C�Y�𒴂��Ă���ꍇ�̓��C�A�E�g���v�Z���Ȃ����D
/// ���q�[�v��alloc��Ԃ̏������D�܂�rp��heapstart,size�̑Ή����Ă�����̂�����heap�ɂ��Ă����������D����rp�ɂ��Ă������C�A�E�g�����D
/// rp�̔ԍ��ɑΉ�������̂��Ȃ��ꍇ�����C�A�E�g�̌v�Z�݂̂�����񂷂�D
/// ���̏ڍׁf�f�f
/// ���̂悤�ɂ���΂�������������layout�����I�ɕς��C�����Ȃ��Drs�͐ÓI�����D
/// �e�e�e
/// ����range���R�s�[�̕K�v�����邩�ǂ����𒲂ׂ�D���C�A�E�g�Čv�Z�̏ꍇ�͖ⓚ���p�őS�R�s�[
/// range���Ƃ̃R�s�[���s���D�T�C�Y��heapstart�͎��Oh�Ɍv�Z�������́D�R�s�[���͒��ׂ�΂킩��D
/// 
/// ���C�A�E�g�����肷��֐����l����D�q�[�v�R�s�[�����łɍs���̂ł�����֐����ŌĂԁD
/// �R�s�[�Ŏ󂯎��x�N�^�[�Ƃ��ꂼ��ǂ�range���ǂ�heaptype�Ȃ̂��Ƃ������
/// heaptype����q�[�v��I�ԁD
/// �q�[�v����T�C�Y���̃��C�A�E�g���擾�D�܂�q�[�v�̐��ł��̕��̃T�C�Y���m�ۂ���allocator������
/// �Ԃ�l�Ƃ��ăT�C�Y�Cheapstart�����炦��͂��Ȃ̂ŕۑ����Ă����D
/// �R�s�[���s���D
/// 
/// </summary>
class GlobalDescriptorHeap
{
public:
	GlobalDescriptorHeap()
	{}
	~GlobalDescriptorHeap() = default;
	DECLMOVECOPY(GlobalDescriptorHeap);

	//�A���P�[�^�[�̏�����
	bool Init(
		D3DDevice* _pDev,
		GlobalDescriptorHeapDesc const&);
	
	//RP��Ή�����q�[�v�ɃR�s�[
	bool CopyToHeapChank(class D3DDevice* _pDev,
		struct RootParameterCopyDesc const& _handleSet, uint32_t _rpIdx, RPBufferType const _type);
	//RP�̏���o�^�D���������Ɏg���q�o�͂����œo�^���Ă����D
	bool RegisterRPChankInfo(uint32_t _rpIdx, RPBufferType _type, size_t numDescriptors);
	//RP���Ή�����q�[�v�̏ꏊ��Ԃ��D
	DescriptorChankInfo const& GetRPDescriptorHeapChank(uint32_t _rpIdx)const;
	std::vector<ID3D12DescriptorHeap*>
		GetHeaps()const {
		std::vector<ID3D12DescriptorHeap*> ret;
		for (auto const& heap : m_descChankAllocators) {
			ret.emplace_back(heap.second->GetHeap());
		}
		assert(ret.size() <= 2);
		return ret;
	}
	auto GetRPChankNum()const {
		return m_rpChankInfo.size();
	}
private:
	std::unordered_map<RPBufferType, unqPtr<DescriptorChankAllocator>>
		m_descChankAllocators;
	std::vector<RootParameterChankInfo> m_rpChankInfo;

	bool InitDescChankAllocator(D3DDevice* _pDev,
		DescriptorAllocatorDesc const&,
		RPBufferType _type);


	RPBufferType ToHeapType(D3D12_DESCRIPTOR_HEAP_TYPE const _type);
};
}

