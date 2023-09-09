#pragma once
#include <cstdint>
#include <Windows.h>
#include "D3D12FrameWork/ComPtr.h"
#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/App.h"
#include "D3D12FrameWork/CommandList.h"
#include "D3D12FrameWork/VertexBuffer.h"
#include "D3D12FrameWork/RootSignature.h"
#include "D3D12FrameWork/GraphicPipelineStateObject.h"
#include "D3D12FrameWork/RenderComponent.h"
#include "D3D12FrameWork/Vector.h"
#include <array>

namespace D3D12FrameWork {
	class VertexBuffer;
	struct BasicVertex
	{
		D3D12FrameWork::Vector4 Position;
		D3D12FrameWork::Vector4 Color;
		BasicVertex(D3D12FrameWork::Vector4 const& pos,
			D3D12FrameWork::Vector4 const& col)
			:Position(pos)
			, Color(col) {}
	};
	class TestApp
		:public App
	{
	public:


		TestApp(uint32_t width, uint32_t height)
			:App(width, height)
		,m_pso("normal") {}
		~TestApp();
		void Run();
		bool Init()override;
		void MainLoop()override;
	private:


		void Term()override;
		bool InitWindow();
		void TermWindow();

		bool InitD3D();

		void Draw();

		void BeginFrame();

		//ウィンドウのプロシージャ関数
		static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

		std::array<CommandList, DX12Settings::BUFFER_COUNT> m_mainCommandLists;
		unqPtr<VertexBuffer> m_pVB;
		GraphicPipelineStateObject<ShaderBlob::VERTEX, ShaderBlob::PIXEL> m_pso;
		RootSignature m_rootSig;
		unqPtr<RenderComponent> m_renderComponent;
	};

}