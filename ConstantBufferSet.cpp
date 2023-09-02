#include "D3D12FrameWork/ConstantBufferSet.h"
#include "D3D12FrameWork/D3DDevice.h"

namespace D3D12FrameWork{
	ConstantBufferSet::ConstantBufferSet()
		:m_pBuffers()
		, m_structDescs()
	{}
	ConstantBufferSet::~ConstantBufferSet(){}
	//registerdescに入っている分のcbを作る．
	//実際使用する際はcbsetはディスクリプタレンジのcbの部分をまとめて管理するため
	//regdescにはrange内のregister情報が入っている
	bool 
		ConstantBufferSet::InitFromRegisterDescWithRange(
			std::unordered_map<std::string, ShaderRegisterDescs::ConstantBindDesc>const& _cbMap,
			std::vector<ShaderRegisterDescs::RegisterDesc> const& _regDesc,
			UINT _rangeStart,
			UINT _rangeNum,
			BufferType _type,
			D3DDevice* _pDev,
			uint32_t _bufferCount) {
		if (_pDev == nullptr) {
			return false;
		}
		m_pBuffers.resize(_bufferCount);



		m_structDescs.clear();
		m_orderedCBInfo.resize(_rangeNum);
		size_t total_size = 0;

		//レジスタの順番と名前&arrayIdxの対応を覚えておく
		std::vector<std::pair<std::string_view, size_t>> regNames; regNames.resize(_rangeNum);
		for (auto& bReg : _regDesc) {
			//範囲内なら記録しておく．
			m_structDescs.emplace(bReg.Name, ConstantStructInfo(
				total_size,
				&_cbMap.at(bReg.Name).VariableDesc
			));
			auto const resSize = _cbMap.at(bReg.Name).Size * bReg.BindCount;
			total_size += (resSize + 0xff) & ~0xff;
			for (int i = 0; i < bReg.BindCount; i++) {
				regNames[bReg.BindPoint - _rangeStart + i] = std::make_pair(std::string_view(bReg.Name), i);
			}
		}

		//まとめてバッファを作る
		for (auto& pBuffer : m_pBuffers) {
			pBuffer.reset(new Buffer());
			if (total_size != 0
				&& !pBuffer->Init(_pDev, total_size, _type)) {
				assert(false);
				m_structDescs.clear();
				return false;
			}
		}


		//バッファに対応するレンジを作成
		for (auto rangeIdx = 0; rangeIdx < _rangeNum; rangeIdx++) {
			if (regNames[rangeIdx].first.empty()) {
				//データが存在しない
				m_orderedCBInfo[rangeIdx] = nullptr;
				continue;
			}
			//バッファ事にレンジのビューを作る。
			auto& structDesc = m_structDescs[std::string(regNames[rangeIdx].first.data())];
			structDesc.CBVs.resize(m_pBuffers.size());
			auto offset = structDesc.Offset +
				_cbMap.at(std::string(regNames[rangeIdx].first.data())).Size * regNames[rangeIdx].second;
			for (auto frameIdx = 0u; frameIdx < m_pBuffers.size(); frameIdx++) {
				if (!m_structDescs[std::string(regNames[rangeIdx].first.data())].CBVs[frameIdx].Init(
					_pDev,
					m_pBuffers[frameIdx].get(),
					offset,
					_cbMap.at(std::string(regNames[rangeIdx].first.data())).Size
				)) {
					assert(false);
					return false;
				}
			}
			
			m_orderedCBInfo[rangeIdx] = &m_structDescs[std::string(regNames[rangeIdx].first.data())];
		}
		
		//UpdateDescriptorHeap();
		return true;
	}

	uint8_t* ConstantBufferSet::MappedPtr(std::string_view _regName, unsigned int _frameIdx) {
		auto pBuffer = m_pBuffers[GetNowFrameIndex()].get();
		if (pBuffer->GetType() != BufferType::BUFFER_TYPE_UPLOAD) {
			assert(false);
			return nullptr;
		}
		auto mapped_start = pBuffer->MappedPtr<uint8_t>();
		auto const offset = m_structDescs[std::string(_regName.begin(), _regName.end())].Offset;
		return mapped_start + offset;
	}

	bool 
		ConstantBufferSet::CopyToResource(D3DDevice* _pdev, CommandList* _pcmdList,
		std::string_view _regName, uint8_t const* _pData, size_t _size, size_t _offset_in_struct,
			bool _copyToBackBuffers) {
		Present();//内容を変更するので読んでおく。
		auto pBuffer = GetNowBuffer();
		if (pBuffer->GetType() != BufferType::BUFFER_TYPE_DEFAULT) {
			assert(false);
			return false;
		}
		if (_pdev == nullptr || _pcmdList == nullptr) {
			assert(false);
			return false;
		}
		
		auto const offset = m_structDescs[std::string(_regName.begin(), _regName.end())].Offset+_offset_in_struct;
		
		if (!_copyToBackBuffers) {
			return pBuffer->CopyToResource(_pdev, _pcmdList, _pData, _size, offset);
		}
		else {
			for (auto& pCB : m_pBuffers) {
				if (!pCB->CopyToResource(_pdev, _pcmdList, _pData, _size, offset)) {
					assert(false);
					return false;
				}
			}
		}
		return true;
	}

	bool
		ConstantBufferSet::CopyToMappedPtr(
			std::string_view _regName, uint8_t const* _pData, size_t _size
			, size_t _offset_in_struct, bool copyToBackBuffers
		) {
		Present();//どうせ変更するのだろうから呼んでおく。
		if (!copyToBackBuffers) {
			auto nowIdx = GetNowFrameIndex();
			std::memcpy(static_cast<void*>(MappedPtr(_regName, nowIdx) + _offset_in_struct),
				static_cast<void const*>(_pData),
				_size);
		}
		else {
			//全部のバッファにコピー
			for (auto i = 0u; i < m_pBuffers.size(); i++) {
				std::memcpy(static_cast<void*>(MappedPtr(_regName, i) + _offset_in_struct),
					static_cast<void const*>(_pData),
					_size);
			}
		}

		return true;
	}
}
