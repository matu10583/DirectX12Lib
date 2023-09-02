#pragma once
#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/CommandList.h"


namespace D3D12FrameWork {
	class D3DDevice;
	class RPBufferController
	{
	public:
		RPBufferController() = delete;
		RPBufferController(class RootParameterBuffer&,
			D3DDevice* const);
		~RPBufferController() = default;
		DECLMOVECOPY(RPBufferController);

		bool SetTexture(
			//お名前とファイル名．ローダーを別クラスで作っておく
			std::string_view regName,
			std::filesystem::path const& _fileName,
			uint32_t const mipL=1
		);
		bool SetSampler(
			std::string_view regName,
			D3D12_SAMPLER_DESC const& _smpDesc
		);
		//isDefaultHeapはheapの種類．普通constantならfalse
		template<typename T>
		bool SetStruct(std::string_view regName, T const& _const, bool _isDefaultHeap = false) {
			auto cb = m_rpBuff.get().FindCBufferFromStructName(regName);
			if (cb == nullptr) {
				assert(false);
				return false;
			}
			if (_isDefaultHeap) {
				m_cmdList.Begin();
				if (!cb->CopyToResource(
					m_pRefDev, &m_cmdList, regName,
					reinterpret_cast<uint8_t const*>(&_const), sizeof(T)
				)) {
					return false;
				}
				CommandList* pCmdList[] = { &m_cmdList };
				m_pRefDev->EndAndExecuteCommandList(pCmdList, 1);
			}
			else {
				std::memcpy(static_cast<void*>(cb->MappedPtr(regName)),
					static_cast<void const*>(&_const),
					sizeof(T));
			}
			return true;
		}
		template<typename T>
		bool SetVariable(std::string_view regName, T const& _const,
			bool setToAllBuffer=false,
			bool _isDefaultHeap = false) {
			size_t offset = 0;
			auto cb = m_rpBuff.get().FindCBufferFromVariableName(regName, &offset);
			if (cb == nullptr) {
				assert(false);
				return false;
			}
			if (_isDefaultHeap) {
				m_cmdList.Begin();
				if (!cb->CopyToResource(
					m_pRefDev, &m_cmdList, regName,
					reinterpret_cast<uint8_t const*>(&_const), sizeof(T),
					offset,setToAllBuffer
				)) {
					return false;
				}
				CommandList* pCmdList[] = { &m_cmdList };
				m_pRefDev->EndAndExecuteCommandList(pCmdList, 1);
			}
			else {
				if (!cb->CopyToMappedPtr(
					regName, reinterpret_cast<uint8_t const*>(&_const),
					sizeof(T), offset, setToAllBuffer
				)) {
					assert(false);
					return false;
				}
			}
			return true;
		}

		void AfterDraw();

	private:
		std::reference_wrapper<RootParameterBuffer> const m_rpBuff;
		D3DDevice* const m_pRefDev;
		CommandList m_cmdList;
	};
}
