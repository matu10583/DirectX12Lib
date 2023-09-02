#include "D3D12FrameWork/MaterialSet.h"
#include "D3D12FrameWork/MaterialFactory.h"

namespace D3D12FrameWork{
	MaterialSet::~MaterialSet() {
		auto ins = MaterialFactory::Instance();
		if (ins == nullptr) {//もう向こうで解放しているはず
			return;
		}
		for (auto idx : m_lambdaIdx) {
			auto pRpBuff = m_materialViews[idx].GetRPBuff();
			ins->DeleteLambdaRPBuffer(pRpBuff);//無名は使いまわししないのでMaterialSetがなくなった時点で殺す
			//無名は生成タイミングもマテリアルセットを作った時なのでバランスがいいはず
			//もしAllocatorを使うならAllocatorに任せるのがいい。
			//(そもそもMaterialSetのみが所有権を持つのが一番いいが、他のRPBufferはMaterialFactoryが所有権をもつ関係上、それらとの区別を付けるのが難しい。)
		}
	}
}
