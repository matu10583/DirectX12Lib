#pragma once
#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/RPBufferController.h"
#include "D3D12FrameWork/RPBufferView.h"

namespace D3D12FrameWork {
	struct MaterialDescriptor
	{
		UINT RPIdx;
		std::string MaterialName;
	};
	struct MaterialSetDescriptor
	{
		std::vector<MaterialDescriptor> MaterialDescs;
		//0ならindicesを使わずにvertex全部を一気に描画する
		uint32_t NumIndices = 0;
	};
	class RPBufferController;
	class RPBufferView;
	//マテリアルの詳細はとにかくvectorを持てばいいかなという感じはあるけどそれとは別にpsoも指定しなきゃいけない．PDOも名前つけた方がわかりやすいよなー
	class MaterialSet
	{
		friend class MaterialFactory;
	public:
		MaterialSet() = default;
		~MaterialSet();
		MaterialSet(MaterialSet&&) = delete;
		MaterialSet(MaterialSet const&) = delete;
		//intで指定してマテリアルを撮ってくるけど使用時はrsのenumとかで撮ってくる方がいいよね
		//ディスクリプタを設定して初期化する方がいいなー
		RPBufferController& GetController(uint32_t _rpIdx) {
			if (_rpIdx >= m_materialControllers.size()) {
				assert(false);
			}
			return m_materialControllers[_rpIdx];
		}
		RPBufferView& GetView(uint32_t _rpIdx) {
			if (_rpIdx >= m_materialViews.size()) {
				assert(false);
			}
			return m_materialViews[_rpIdx];
		}
		size_t Size() const {
			assert(m_materialControllers.size() == m_materialViews.size());
			return m_materialViews.size();
		}
		uint32_t IndicesNum()const {
			return m_indicesNum;
		}
		void SetIndicesNum(uint32_t const _indicesNum) {
			m_indicesNum = _indicesNum;
		}

	private:
		//ここはrpの順番通りに並べる．ここはrpbufferへの参照ではなくhandlerを持たせた方がいい．
		std::vector<RPBufferController> m_materialControllers;
		std::vector<RPBufferView> m_materialViews;
		//lambdaなbufferについては自分で所有権を持つ
		std::vector<unqPtr<RootParameterBuffer>> m_plambdaBuff;
		//インデックス数
		uint32_t  m_indicesNum;
	};
}


