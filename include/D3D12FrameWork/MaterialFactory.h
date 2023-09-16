#pragma once
#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/ComPtr.h"
#include "D3D12FrameWork/RootParameterBuffer.h"
#include <unordered_map>
#include <deque>
#include "D3D12FrameWork/MaterialSet.h"
#include "D3D12FrameWork/LambdaRPBuffer.h"
namespace D3D12FrameWork {
	/*
	�g�p���@
	device�ŏ���������D
	�ŏ���pso���ƂɃ}�e���A��������Ă����D�f�t�H���g�}�e���A�����ŏ��ɐݒ肵�Ă����D
		���̊֐�����rootparameterbuff�̏��������s����D
			�܂肻�̐�Ŋj�탌���W�̃o�b�t�@�������������͂�
	creatematerialControllerSet���Ăяo���ă}�e���A���Z�b�g�����D
		���ۂɃ}�e���A���̖��O�Ǝg��pso���L�q����K�v������D
		��������Ƃ�controller������ĕԂ��D���̃R���g���[���[��rendercomp�݂����Ȃ̂����Ƃ悢�D
	*/

	class MaterialFactory
	{
	public:
		static bool Create(class D3DDevice* _pDev) {
			if (m_instance != nullptr) {
				return false;
			}
			m_instance =new MaterialFactory;
			m_instance->m_pRefDev = _pDev;
			return true;
		}
		static void Destroy() {
			delete m_instance;
			m_instance = nullptr;
		}
		static MaterialFactory* Instance() {
			return m_instance;
		}

		//���̂͂܂Ƃ߂čŏ��̕��ɍ��D������}�e���A����map�ŕۑ��D
		bool CreateMaterial(
			std::string_view _matName,//���}�e���A���̖��O
			class IPipelineStateObject* _pso,//�ΏۂƂȂ�pso
			UINT _rpIdx,//����pso�̉��Ԗڂ�rp�̃}�e���A�������̂�
			bool isDefault = false,//�}�e���A���Z�b�g�쐬���ɒl���w�肳��Ă��Ȃ��ꍇ�͂��̃}�e���A�����f�t�H���g�Ƃ���̂�
			uint32_t _bufferCount=DX12Settings::BUFFER_COUNT
		);

		//�쐬�ς݂�rootparameterbuff��g�ݍ��킹�ă}�e���A���Z�b�g�����D
		unqPtr<class MaterialSet> CreateMaterialSet(
			MaterialSetDescriptor const& _matSetDesc,
			class IPipelineStateObject const* _pso,
			uint32_t _lambdaBufferCount = DX12Settings::BUFFER_COUNT
		);

		RPBufferController GetController(std::string_view _psoName, std::string_view _matName);
		RPBufferView GetView(std::string_view _psoName, std::string_view _matName);

	private:
		MaterialFactory()
			:m_pRefDev(nullptr) {}
		~MaterialFactory() {}
		DECLMOVECOPY(MaterialFactory);
		static MaterialFactory* m_instance;

		struct RootParameterBuffMap
		{
			std::unordered_map<std::string, unqPtr<RootParameterBuffer>> RPBuffs;
		};
		struct LambdaRPBufferArray{
			std::deque<unqPtr<RootParameterBuffer>> LambdaRPBuffers;
		};

		//�}�e���A�����L�����Ă���.psoName:(MaterialName:rpBuff)
		std::unordered_map<std::string, RootParameterBuffMap> m_psoRpBuffMap;

		D3DDevice* m_pRefDev;

	};

}
