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
		std::string_view _matName,//作るマテリアルの名前
		class IPipelineStateObject* _pso,//対象となるpso
		UINT _rpIdx,//そのpsoの何番目のrpのマテリアルを作るのか
		bool isDefault,//マテリアルセット作成時に値が指定されていない場合はこのマテリアルをデフォルトとするのか
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
		if (rpBMap.count(mat_name)) {//重複を防ぐ
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
			//rp番号のdescriptorを探す．
			auto matDesc = std::find_if(_matSetDesc.MaterialDescs.begin(),
				_matSetDesc.MaterialDescs.end(),
				[i](MaterialDescriptor const& mdesc) {
					return mdesc.RPIdx == i;
				});
			std::string_view matName = {};
			if (matDesc == _matSetDesc.MaterialDescs.end()) {//マテリアルが指定されていないのでデフォルト値
				matName = _pso->GetDefaultMaterialName(i);
			}
			else {//指定された名前
				matName = matDesc->MaterialName;
			}

			RootParameterBuffer* matBuffer = nullptr;

			if (matName.empty()) {
				//無名バッファを作る。
				matBuffer = CreateLambdaRPBuffer(
					_pso, i, _lambdaBufferCount
				);
				ret->m_lambdaIdx.push_back(i);
			}
			else {
				auto psoName = _pso->GetName();
				if (!m_psoRpBuffMap.count(psoName.data())) {
					throw std::runtime_error("指定されたpsoで使用されるマテリアルは一つも登録されていません．");

					//そのpsoのマテリアルは一つもない．
				}
				auto& rpBMap = m_psoRpBuffMap[psoName.data()].RPBuffs;
				if (!rpBMap.count(matName.data())) {
					//未作成のマテリアル
					throw std::runtime_error("未作成のマテリアル");

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
		IPipelineStateObject const* _pso,//対象となるpso
		UINT _rpIdx,//そのpsoの何番目のrpのマテリアルを作るのか
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
