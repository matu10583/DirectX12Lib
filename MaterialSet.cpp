#include "D3D12FrameWork/MaterialSet.h"
#include "D3D12FrameWork/MaterialFactory.h"

namespace D3D12FrameWork{
	MaterialSet::~MaterialSet() {
		auto ins = MaterialFactory::Instance();
		if (ins == nullptr) {//�����������ŉ�����Ă���͂�
			return;
		}
		for (auto idx : m_lambdaIdx) {
			auto pRpBuff = m_materialViews[idx].GetRPBuff();
			ins->DeleteLambdaRPBuffer(pRpBuff);//�����͎g���܂킵���Ȃ��̂�MaterialSet���Ȃ��Ȃ������_�ŎE��
			//�����͐����^�C�~���O���}�e���A���Z�b�g����������Ȃ̂Ńo�����X�������͂�
			//����Allocator���g���Ȃ�Allocator�ɔC����̂������B
			//(��������MaterialSet�݂̂����L�������̂���Ԃ������A����RPBuffer��MaterialFactory�����L�������֌W��A�����Ƃ̋�ʂ�t����̂�����B)
		}
	}
}
