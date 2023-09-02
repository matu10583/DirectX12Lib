#include "D3D12FrameWork/SamplerSet.h"


namespace D3D12FrameWork{
	bool 
		SamplerSet::InitFromRegisterDescWithRange(
			std::unordered_map<std::string, ShaderRegisterDescs::SamplerBindDesc>const& _smpDesc,
			std::vector<ShaderRegisterDescs::RegisterDesc> const& _regDesc,
			UINT _rangeStart,
			UINT _rangeNum,
			class D3DDevice* _pDev) {
		if (_pDev == nullptr) {
			return false;
		}

		//レジスタの順番と名前&arrayIdxの対応を覚えておく
		std::vector<std::pair<std::string_view, size_t>> regNames; regNames.resize(_rangeNum);
		for (auto const& sReg : _regDesc) {
			for (int i = 0; i < sReg.BindCount; i++) {
				regNames[sReg.BindPoint - _rangeStart + i] = std::make_pair(std::string_view(sReg.Name), i);
			}
		}
		m_samplerViewRef.resize(_rangeNum);
		for (auto i = 0; i < _rangeNum; i++) {
			if (regNames[i].first.empty()) {
				//データが存在しない
				m_samplerViewRef[i] = nullptr;
				continue;
			}
			//samplerとそのビューを作る
			auto smpName = std::string(regNames[i].first.data());
			if (!m_samplerMap[smpName].Init(_pDev
			)) {
				assert(false);
				return false;
			}
			m_samplerViewRef[i] = &m_samplerMap[smpName];
		}
		//UpdateDescriptorHeap();
		return true;
	}
	bool 
		SamplerSet::CreateSampler(D3DDevice* _pdev,
		std::string_view _regName,
		D3D12_SAMPLER_DESC const& _smpDesc) {
		if (!m_samplerMap.count(_regName.data())) {
			assert(false);
			return false;
		}
		if (m_samplerMap[_regName.data()].UpdateSampler(_pdev, _smpDesc)) {
			//UpdateDescriptorHeap();
			return true;
		}
		return false;
	}
}
