#pragma once
#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/ComPtr.h"
#include "D3D12FrameWork/MeshBuffer.h"
#include "D3D12FrameWork/MeshSet.h"
#include <unordered_map>
//#include "D3D12FrameWork/MeshSet.h"
namespace D3D12FrameWork {
	/*
	�g�p���@
	device�ŏ���������D
	�ŏ���pso���ƂɃ��b�V��������Ă����D�f�t�H���g���b�V�����ŏ��ɐݒ肵�Ă����D
		���̊֐�����rootparameterbuff�̏��������s����D
			�܂肻�̐�Ŋj�탌���W�̃o�b�t�@�������������͂�
	creatematerialControllerSet���Ăяo���ă��b�V���Z�b�g�����D
		���ۂɃ��b�V���̖��O�Ǝg��pso���L�q����K�v������D
		��������Ƃ�controller������ĕԂ��D���̃R���g���[���[��rendercomp�݂����Ȃ̂����Ƃ悢�D
	*/

	class MeshFactory
	{
	public:
		static bool Create(class D3DDevice* _pDev) {
			if (m_instance != nullptr) {
				return false;
			}
			m_instance = new MeshFactory;
			m_instance->m_pRefDev = _pDev;
			return true;
		}
		static void Destroy() {
			delete m_instance;
		}
		static MeshFactory* Instance() {
			return m_instance;
		}

		//���̂͂܂Ƃ߂čŏ��̕��ɍ��D��������b�V����map�ŕۑ��D
		MeshFactory* CreateMesh(
			std::string_view _matName,//��郁�b�V���̖��O
			class IPipelineStateObject* _pso,//�ΏۂƂȂ�pso
			bool useIndex = false
		);
		MeshBufferController MeshController(
			std::string_view _matName,
			class IPipelineStateObject const* _pso
		);

		//�쐬�ς݂�mesh�ă��b�V���Z�b�g�����D
		unqPtr<class MeshSet> CreateMeshSet(
			std::string_view const _meshName,
			class IPipelineStateObject const* _pso
		);

	private:
		MeshFactory()
			:m_pRefDev(nullptr) {}
		~MeshFactory() {}
		DECLMOVECOPY(MeshFactory);
		static MeshFactory* m_instance;

		struct MeshBufferMap
		{
			std::unordered_map<std::string, unqPtr<MeshBuffer>> RPBuffs;
		};

		//���b�V�����L�����Ă���.psoName:(MeshName:rpBuff)
		std::unordered_map<std::string, MeshBufferMap> m_psoMeshBuffMap;
		D3DDevice* m_pRefDev;
	};

}
