#pragma once
#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/ComPtr.h"
#include "D3D12FrameWork/MeshBuffer.h"
#include "D3D12FrameWork/MeshSet.h"
#include <unordered_map>
//#include "D3D12FrameWork/MeshSet.h"
namespace D3D12FrameWork {
	/*
	使用方法
	deviceで初期化する．
	最初にpsoごとにメッシュを作っておく．デフォルトメッシュも最初に設定しておく．
		この関数内でrootparameterbuffの初期化が行われる．
			つまりその先で核種レンジのバッファが初期化されるはず
	creatematerialControllerSetを呼び出してメッシュセットを作る．
		作る際にメッシュの名前と使うpsoを記述する必要がある．
		それをもとにcontrollerを作って返す．このコントローラーはrendercompみたいなのが持つとよい．
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

		//作るのはまとめて最初の方に作る．作ったメッシュはmapで保存．
		MeshFactory* CreateMesh(
			std::string_view _matName,//作るメッシュの名前
			class IPipelineStateObject* _pso,//対象となるpso
			bool useIndex = false
		);
		MeshBufferController MeshController(
			std::string_view _matName,
			class IPipelineStateObject const* _pso
		);

		//作成済みのmeshてメッシュセットを作る．
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

		//メッシュを記憶しておく.psoName:(MeshName:rpBuff)
		std::unordered_map<std::string, MeshBufferMap> m_psoMeshBuffMap;
		D3DDevice* m_pRefDev;
	};

}
