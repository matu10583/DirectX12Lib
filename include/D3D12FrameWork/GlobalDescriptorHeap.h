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
/// 全ディスクリプタひーぶをここにコピーしてそれをpsoに設定する．そのためのやつ
/// rpのレイアウトはほとんど変わらないはず．先にレイアウトを設定して後からコピーするのが計算量的には楽
/// レイアウトといってもrpのサイズくらいしか覚えたくない．rpの番号とheapstartとの対応を覚えておくとよいと思う．
/// コピーするときを考える．rpに含まれるviewのそれぞれの場所はわかるはず．viewのサイズはdescのtypeが分かれば大丈夫
/// コピー関数は一つのタイプずつに実行する必要がある．
/// そしたらrangeごとに受け取らなければならない．
/// rpBufferからorderedRangeSetを取得する．それぞれでGetViewをすることによってIresourceViewの配列を得られる．
/// コピーをどの単位で行うか．とりあえずrangeごとのコピーでいいんじゃないか．
/// 一応レジスタ単位でのコピーの可否も出来なくはないけどrpが今のところ区切りになっている．
/// つまりrpに一緒に含まれるパラメーターは一緒に変えられる可能性が高いパラメーター，
/// そう考えるとrpでまとめてコピーしてもいいくらいだが，rangeごとにコピー関数が走るので
/// rangeごとに決定した方が楽，まあ上述の理由からそれで十分だろうというところです．
/// 今のところおそらくrangeのheapは連続なのでsimpleを使ってもいいけどかならず連続になるわけでないのでとりあえず普通の関数を使う．
/// 
/// rpごとにヒープにコピーする関数を考える．rpに対応したheapstartとサイズは覚えておく．
/// 事前にコピーする必要があるのかはフラグで調べておく．
/// 入力としては((cpuhandleのvector)のrangeごとのvector)例えばcbv,srvではcbvのhandleのベクター，srvのハンドルのベクター.rangeの順番通り
/// あとrangeごとにコピーする必要があるのか
/// サイズのチェック．vectorの総数からサイズを計算する．サイズを超えている場合はレイアウトを計算しなおす．
/// ↑ヒープのalloc状態の初期化．つまりrpとheapstart,sizeの対応しているものをこのheapについてだけ初期化．今のrpについてだけレイアウトを作る．
/// rpの番号に対応するものがない場合もレイアウトの計算のみもう一回する．
/// ↑の詳細’’’
/// そのようにすればいいかもだけどlayoutが動的に変わる気がしない．rsは静的だし．
/// ‘‘‘
/// そのrangeがコピーの必要があるかどうかを調べる．レイアウト再計算の場合は問答無用で全コピー
/// rangeごとのコピーを行う．サイズとheapstartは事前hに計算したもの．コピー元は調べればわかる．
/// 
/// レイアウトを決定する関数を考える．ヒープコピーもついでに行うのでそれも関数内で呼ぶ．
/// コピーで受け取るベクターとそれぞれどのrangeがどのheaptypeなのかという情報
/// heaptypeからヒープを選ぶ．
/// ヒープからサイズ分のレイアウトを取得．つまりヒープの数でその分のサイズを確保するallocatorを書く
/// 返り値としてサイズ，heapstartをもらえるはずなので保存しておく．
/// コピーを行う．
/// 
/// </summary>
class GlobalDescriptorHeap
{
public:
	GlobalDescriptorHeap()
	{}
	~GlobalDescriptorHeap() = default;
	DECLMOVECOPY(GlobalDescriptorHeap);

	//アロケーターの初期化
	bool Init(
		D3DDevice* _pDev,
		GlobalDescriptorHeapDesc const&);
	
	//RPを対応するヒープにコピー
	bool CopyToHeapChank(class D3DDevice* _pDev,
		struct RootParameterCopyDesc const& _handleSet, uint32_t _rpIdx, RPBufferType const _type);
	//RPの情報を登録．初期化時に使うＲＰはここで登録しておく．
	bool RegisterRPChankInfo(uint32_t _rpIdx, RPBufferType _type, size_t numDescriptors);
	//RPが対応するヒープの場所を返す．
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

