#include "D3D12FrameWork/MaterialFactory.h"
#include "D3D12FrameWork/RPBufferController.h"
#include "D3D12FrameWork/RPBufferView.h"
#include <algorithm>
#include <stdexcept>
#include "D3D12FrameWork/CommandList.h"
#include "D3D12FrameWork/D3DDevice.h"

namespace D3D12FrameWork{
	MaterialFactory* MaterialFactory::m_instance = nullptr;

	bool 
		MaterialFactory::CreateMaterial(
		std::string_view _matName,//���}�e���A���̖��O
		class IPipelineStateObject* _pso,//�ΏۂƂȂ�pso
		UINT _rpIdx,//����pso�̉��Ԗڂ�rp�̃}�e���A�������̂�
		bool isDefault,//�}�e���A���Z�b�g�쐬���ɒl���w�肳��Ă��Ȃ��ꍇ�͂��̃}�e���A�����f�t�H���g�Ƃ���̂�
		uint32_t _bufferCount
		) {
		auto const& rprDesc = _pso->GetRPRegisterDesc(_rpIdx);
		auto const& rsRpDesc = _pso->GetRootSignature()->GetRootParamDescs();
		auto tmpRPB = std::make_unique<RootParameterBuffer>();
		if (!tmpRPB->Init(rprDesc, rsRpDesc[_rpIdx], m_pRefDev, _bufferCount)) {
			assert(false);
			return false;
		}
		auto pso_name = _pso->GetName();
		auto mat_name = std::string(_matName.data());
		auto& rpBMap = m_psoRpBuffMap[pso_name.data()].RPBuffs;
		if (rpBMap.count(mat_name)) {//�d����h��
			assert(false);
			return false;
		}
		rpBMap[mat_name].reset(tmpRPB.release());

		if (isDefault) {
			_pso->SetDefaultMaterial(_matName, _rpIdx);
		}

		return true;
	}

	unqPtr<MaterialSet> 
		MaterialFactory::CreateMaterialSet(
			MaterialSetDescriptor const& _matSetDesc,
			IPipelineStateObject const* _pso,
			uint32_t _lambdaBufferCount
		) {
		auto ret = std::make_unique<MaterialSet>();
		auto rpNum = _pso->GetRootSignature()->GetRootParamDescs().size();
		ret->m_materialControllers.clear();
		ret->m_materialControllers.reserve(rpNum);

		for (uint32_t i = 0; i < rpNum; i++) {
			//rp�ԍ���descriptor��T���D
			auto matDesc = std::find_if(_matSetDesc.MaterialDescs.begin(),
				_matSetDesc.MaterialDescs.end(),
				[i](MaterialDescriptor const& mdesc) {
					return mdesc.RPIdx == i;
				});
			std::string_view matName = {};
			if (matDesc == _matSetDesc.MaterialDescs.end()) {//�}�e���A�����w�肳��Ă��Ȃ��̂Ńf�t�H���g�l
				matName = _pso->GetDefaultMaterialName(i);
			}
			else {//�w�肳�ꂽ���O
				matName = matDesc->MaterialName;
			}

			RootParameterBuffer* matBuffer = nullptr;

			if (matName.empty()) {
				//�����o�b�t�@�����B
				matBuffer = CreateLambdaRPBuffer(
					_pso, i, _lambdaBufferCount
				);
				ret->m_lambdaIdx.push_back(i);
			}
			else {
				auto psoName = _pso->GetName();
				if (!m_psoRpBuffMap.count(psoName.data())) {
					throw std::runtime_error("�w�肳�ꂽpso�Ŏg�p�����}�e���A���͈���o�^����Ă��܂���D");

					//����pso�̃}�e���A���͈���Ȃ��D
				}
				auto& rpBMap = m_psoRpBuffMap[psoName.data()].RPBuffs;
				if (!rpBMap.count(matName.data())) {
					//���쐬�̃}�e���A��
					throw std::runtime_error("���쐬�̃}�e���A��");

				}
				matBuffer = rpBMap[matName.data()].get();
			}
			

			ret->m_materialControllers.emplace_back(
				RPBufferController(
					*matBuffer, m_pRefDev)
			);
			ret->m_materialViews.emplace_back(
				RPBufferView(
					*matBuffer
				)
			);
		}
		return ret;
	}

	RootParameterBuffer*
		MaterialFactory::CreateLambdaRPBuffer(
		IPipelineStateObject const* _pso,//�ΏۂƂȂ�pso
		UINT _rpIdx,//����pso�̉��Ԗڂ�rp�̃}�e���A�������̂�
		uint32_t _bufferCount
	) {
		m_lambdaRPBuffers.LambdaRPBuffers
			.emplace_back(std::make_unique<RootParameterBuffer>());
		auto rpBuff = m_lambdaRPBuffers.LambdaRPBuffers.back().get();
		auto const& rprDesc = _pso->GetRPRegisterDesc(_rpIdx);
		auto const& rsRpDesc = _pso->GetRootSignature()->GetRootParamDescs();
		if (!rpBuff->Init(rprDesc, rsRpDesc[_rpIdx], m_pRefDev, _bufferCount)) {
			assert(false);
			return nullptr;
		}
		return rpBuff;
	}

	void
		MaterialFactory::DeleteLambdaRPBuffer(
			RootParameterBuffer* _deletedBuffer
		) {
		auto it = std::find_if(m_lambdaRPBuffers.LambdaRPBuffers.begin(),
			m_lambdaRPBuffers.LambdaRPBuffers.end(),
			[_deletedBuffer](auto const& ptr) {
				return ptr.get() == _deletedBuffer;
			});
		if (it != m_lambdaRPBuffers.LambdaRPBuffers.end()) {
			m_lambdaRPBuffers.LambdaRPBuffers.erase(it);
		}
	}
}