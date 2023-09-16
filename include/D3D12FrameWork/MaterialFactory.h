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
	使用方法
	deviceで初期化する．
	最初にpsoごとにマテリアルを作っておく．デフォルトマテリアルも最初に設定しておく．
		この関数内でrootparameterbuffの初期化が行われる．
			つまりその先で核種レンジのバッファが初期化されるはず
	creatematerialControllerSetを呼び出してマテリアルセットを作る．
		作る際にマテリアルの名前と使うpsoを記述する必要がある．
		それをもとにcontrollerを作って返す．このコントローラーはrendercompみたいなのが持つとよい．
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

		//作るのはまとめて最初の方に作る．作ったマテリアルはmapで保存．
		bool CreateMaterial(
			std::string_view _matName,//作るマテリアルの名前
			class IPipelineStateObject* _pso,//対象となるpso
			UINT _rpIdx,//そのpsoの何番目のrpのマテリアルを作るのか
			bool isDefault = false,//マテリアルセット作成時に値が指定されていない場合はこのマテリアルをデフォルトとするのか
			uint32_t _bufferCount=DX12Settings::BUFFER_COUNT
		);

		//作成済みのrootparameterbuffを組み合わせてマテリアルセットを作る．
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

		//マテリアルを記憶しておく.psoName:(MaterialName:rpBuff)
		std::unordered_map<std::string, RootParameterBuffMap> m_psoRpBuffMap;

		D3DDevice* m_pRefDev;

	};

}
