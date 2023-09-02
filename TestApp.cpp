#include "D3D12FrameWork/TestApp.h"
#include "D3D12FrameWork/D3DDevice.h"
#include "D3D12FrameWork/CommandList.h"
#include "D3D12FrameWork/CommandQueue.h"
#include "D3D12FrameWork/SwapChain.h"
#include "D3D12FrameWork/Texture.h"
#include "D3D12FrameWork/TextureView.h"
#include "D3D12FrameWork/Fence.h"
#include "D3D12FrameWork/ShaderSet.h"
#include "D3D12FrameWork/DXCompiler.h"
#include "D3D12FrameWork/MaterialFactory.h"
#include "D3D12FrameWork/MaterialSet.h"
#include "D3D12FrameWork/RPBufferController.h"
#include "D3D12FrameWork/RPBufferView.h"
#include "D3D12FrameWork/RenderComponent.h"
#include "D3D12FrameWork/GlobalDescriptorHeap.h"
#include "D3D12FrameWork/MeshFactory.h"
#include "D3D12FrameWork/ModelLoader.h"

#include <cassert>
#include <filesystem>
#include <shlobj.h>
#include <span>

namespace {
	//ウィンドウクラス名
	const auto CLASSNAME = TEXT("MeshRenderSample");
	//ウィンドウ名
	const auto WNDNAME = TEXT("MeshRender");

	//pix on windowsによるデバッグ

#if defined(DEBUG) || defined(_DEBUG)
	static std::wstring GetLatestWinPixGpuCapturerPath_Cpp17()
	{
		LPWSTR programFilesPath = nullptr;
		SHGetKnownFolderPath(FOLDERID_ProgramFiles, KF_FLAG_DEFAULT, NULL, &programFilesPath);

		std::filesystem::path pixInstallationPath = programFilesPath;
		pixInstallationPath /= "Microsoft PIX";

		std::wstring newestVersionFound;

		for (auto const& directory_entry : std::filesystem::directory_iterator(pixInstallationPath))
		{
			if (directory_entry.is_directory())
			{
				if (newestVersionFound.empty() || newestVersionFound < directory_entry.path().filename().c_str())
				{
					newestVersionFound = directory_entry.path().filename().c_str();
				}
			}
		}

		if (newestVersionFound.empty())
		{
			// TODO: Error, no PIX installation found
		}

		return pixInstallationPath / newestVersionFound / L"WinPixGpuCapturer.dll";
	}
#endif // defined(DEBUG) || defined(_DEBUG)
}
namespace D3D12FrameWork {

	TestApp::~TestApp() {

	}

	void TestApp::Run() {
		if (Init()) {
			MainLoop();
		}
		Term();
	}

	bool TestApp::Init() {
		if (!ShaderCompiler::Create(ShaderCompiler::ShaderCompilerType::DXC)) {
			return false;
		}
		if (!InitWindow()) {
			return false;
		}
		if (!InitD3D()) {
			return false;
		}

		return true;
	}

	//ウィンドウの初期化
	bool TestApp::InitWindow() {
		//ハンドルの取得
		auto hnd = GetModuleHandle(nullptr);
		if (hnd == nullptr) {
			return false;
		}

		//ウィンドウクラスの設定
		WNDCLASSEX wc = {};
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = WndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hnd;
		wc.hIcon = LoadIcon(hnd, IDI_APPLICATION);
		wc.hCursor = LoadCursor(hnd, IDC_ARROW);
		wc.hbrBackground = GetSysColorBrush(COLOR_BACKGROUND);
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = CLASSNAME;

		//windowの登録
		if (!RegisterClassEx(&wc)) {
			assert(false);
			return false;
		}
		m_hInst = hnd;

		//ウィンドウサイズ
		RECT rc = {};
		rc.right = static_cast<LONG>(m_width);
		rc.bottom = static_cast<LONG>(m_height);

		//ウィンドウサイズの調整
		auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
		AdjustWindowRect(&rc, style, FALSE);

		//ウィンドウの作成
		m_hwnd = CreateWindowEx(
			0,
			CLASSNAME,
			WNDNAME,
			style,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			rc.right - rc.left,
			rc.bottom - rc.top,
			nullptr,
			nullptr,
			m_hInst,
			nullptr
		);

		if (m_hwnd == nullptr) {
			assert(false);
			return false;
		}

		//ウィンドウの表示
		ShowWindow(m_hwnd, SW_SHOWNORMAL);

		//ウインドウの更新
		UpdateWindow(m_hwnd);
		SetFocus(m_hwnd);

		return true;
	}

	LRESULT CALLBACK
		TestApp::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
		switch (msg)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			break;
		}

		return DefWindowProc(hwnd, msg, wp, lp);
	}

	bool TestApp::InitD3D() {
#if defined(DEBUG) || defined(_DEBUG)
		{
			ComPtr<ID3D12Debug> debug;
			auto hr = D3D12GetDebugInterface(IID_PPV_ARGS(debug.ReleaseAndGetAddressOf()));
			if (SUCCEEDED(hr)) {
				debug->EnableDebugLayer();
			}
		}
		//pixでのデバッグ
		if (GetModuleHandle(L"WinPixGpuCapturer.dll") == 0)
		{
			LoadLibrary(GetLatestWinPixGpuCapturerPath_Cpp17().c_str());
		}

#endif // defined(DEBUG) || defined(_DEBUG)
		//デバイスの作成
		if (!m_device.Init(m_hwnd, m_width, m_height)) {
			assert(false);
			return false;
		}

		//オフラインで決められる程度には基本的な設定なので最初に作る．
		auto smp_desc = D3D12_SAMPLER_DESC{
			.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
			.MinLOD = 0.0f,
			.MaxLOD = D3D12_FLOAT32_MAX
		};
		m_rootSig.AddStaticSampler("g_sampler", 0, smp_desc);
		m_rootSig.Init(std::move(
			Set<ShaderBlob::ShaderType>(ShaderBlob::VERTEX | ShaderBlob::PIXEL)
		), &m_device);
		auto ghdesc = GlobalDescriptorHeapDesc{
			.CBV_SRV_UAV = DescriptorAllocatorDesc{.NumDescriptor = 100},
			.SMP = DescriptorAllocatorDesc{.NumDescriptor = 100}
		};


		//コマンドリストの作製
		for (auto& cmdList : m_mainCommandLists) {
			if (!cmdList.Init(&m_device, m_device.GetCmdQueue()->GetType(), ghdesc)) {
				assert(false);
				return false;
			}
			cmdList.RegisterRootSignature(&m_rootSig);
		}
		//コピーを途中で行うので最後に使うであろうリストを初期化しておく
		auto currentFrameIdx = m_device.GetSwapChain()->GetCurrentFrameIndex();
		auto& nowCmdList = m_mainCommandLists[
			(currentFrameIdx+DX12Settings::BUFFER_COUNT-1)%DX12Settings::BUFFER_COUNT];


		//コマンドリストの実行
		//CommandList* pCmdLists[] = {&nowCmdList};
		//m_device.Signal();	
		//m_device.EndAndExecuteCommandList(pCmdLists, ARRAYSIZE(pCmdLists));

		//m_device.WaitSignal();


		//pso作製
		PipelineStateDesc psDesc{
			.RasterizerState{
				.FillMode = D3D12_FILL_MODE_SOLID,
				.CullMode = D3D12_CULL_MODE_NONE,
				.DepthClipEnable = true,
				.MultisampleEnable = false
			},
			.BlendState{
				.AlphaToCoverageEnable = false,
				.IndependentBlendEnable = false,
			},
			.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
			.SampleDesc{
				.Count = 1,
				.Quality = 0
			}
		};
		psDesc.BlendState.RenderTarget[0].BlendEnable = false;
		psDesc.BlendState.RenderTarget[0].LogicOpEnable = false;
		psDesc.BlendState.RenderTarget[0].RenderTargetWriteMask =
			D3D12_COLOR_WRITE_ENABLE_ALL;

		m_pso.PrepareRootSignature(&m_rootSig);
		m_pso.PrepareShaderAsInput<ShaderBlob::VERTEX>(L"triangle/triangleVS");
		m_pso.PrepareShaderAsOutput<ShaderBlob::PIXEL>(L"triangle/trianglePS");
		
		m_pso.EndPrepareAndInit(std::move(psDesc),&m_device);

		//マテリアルの作製
		MaterialFactory::Create(&m_device);
		auto matFac = MaterialFactory::Instance();
		matFac->CreateMaterial(
			"testMaterial0",
			&m_pso,
			0, false
		);
		matFac->CreateMaterial(
			"testMaterial1",
			&m_pso,
			1, true
		);
		matFac->CreateMaterial(
			"testMaterial2",
			&m_pso,
			2, true
		);
		matFac->CreateMaterial(
			"testMaterial3",
			&m_pso,
			3, true
		);
		matFac->CreateMaterial(
			"testMaterial4",
			&m_pso,
			4, true
		);
		matFac->CreateMaterial(
			"testMaterial5",
			&m_pso,
			5, true
		);


		std::array<MaterialSetDescriptor, 1> matsetdesc;
		m_renderComponent.reset(new RenderComponent(m_pso));
		m_renderComponent->CreateMaterialSet(matsetdesc);


		auto material = m_renderComponent->Material(0);

		material->GetController(0).SetVariable<float>("num", 1.1f,true);
		material->GetController(0).SetTexture("g_texture", "res/texture/Desktop.png");
		//cont.GetHandler(1).SetSampler("g_sampler", smp_desc);


		MeshFactory::Create(&m_device);
		auto meshFac = MeshFactory::Instance();
		meshFac->CreateMesh(
			"test_mesh",
			&m_pso
		);
		meshFac->CreateMesh(
			"test_fbx",
			&m_pso
		);
		auto vertices = std::span<BasicVertex>();
		auto indices = std::span<unsigned short>();
		//ModelLoader<BasicVertex>::Load(
		//	"res/model/ac_guitar.fbx", vertices, indices
		//);


		m_renderComponent->CreateMeshSet("test_mesh");
		auto mesh = m_renderComponent->Mesh();

		//vbの作製(あとで変更する．)
		std::vector<BasicVertex> triangleVertices;
		triangleVertices.emplace_back(
			BasicVertex(Vector4(-1.0f, -1.0f, 0.0f, 1.0f), { 1.0f,0.0f,0.0f,1.0f })
		);
		triangleVertices.emplace_back(
			BasicVertex(Vector4(-1.0f, 1.0f, 0.0f, 1.0f), { 0.0f,0.0f,1.0f,1.0f })
		);
		triangleVertices.emplace_back(
			BasicVertex(Vector4(1.0f, -1.0f, 0.0f, 1.0f), { 0.0f,1.0f,0.0f,1.0f })
		);
		mesh->GetController().SetVertices<BasicVertex>(
			triangleVertices, 0
			);

		return true;
	}

	void TestApp::MainLoop() {
		MSG msg = {};
		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) == TRUE) {
				TranslateMessage(&msg);
				//wndprocに送信
				DispatchMessage(&msg);
			}
			BeginFrame();
			
			Draw();
		}
	}

	void TestApp::TermWindow() {
		if (m_hInst != nullptr) {
			UnregisterClass(CLASSNAME, m_hInst);
		}

		m_hInst = nullptr;
		m_hwnd = nullptr;
	}

	void TestApp::Term() {
		m_pVB.reset();
		ShaderCompiler::Destroy();
		MaterialFactory::Destroy();
		MeshFactory::Destroy();
		TermWindow();
	}

	void TestApp::BeginFrame() {
		m_device.WaitPresent();
		auto currentFrameIdx = m_device.GetSwapChain()->GetCurrentFrameIndex();
		auto& nowCmdList = m_mainCommandLists[currentFrameIdx];
		nowCmdList.Begin();
	}

	void
		TestApp::Draw() {
		m_device.SyncKill();

		auto pCmdQueue = m_device.GetCmdQueue();
		auto currentFrameIdx = m_device.GetSwapChain()->GetCurrentFrameIndex();
		auto& nowCmdList = m_mainCommandLists[currentFrameIdx];


		nowCmdList.TransitState(
			m_device.GetSwapChain()->GetCurrentTexture(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		);
		nowCmdList.SetGraphicPipeline(&m_pso);

		nowCmdList.SetAndClearRenderTargets(
			m_device.GetSwapChain()->GetCurrentRTV(),
			1,
			{1.0,0.25f,0.25f,1.0f}
		);

		auto const& scDesc = m_device.GetSwapChain()->GetCurrentTexture()->GetTextureDesc();
		auto vp = D3D12_VIEWPORT{
			.TopLeftX = 0,
			.TopLeftY = 0,
			.Width = (float)scDesc.width,
			.Height = (float)scDesc.height,
			.MinDepth = 0.0f,
			.MaxDepth = 1.0f
		};
		nowCmdList.SetViewPort(&vp, 1);
		auto sr = D3D12_RECT{
			.left = 0,
			.top = 0,
			.right = (LONG)scDesc.width,
			.bottom = (LONG)scDesc.height
		};
		nowCmdList.SetScissorRect(&sr, 1);

		nowCmdList.Draw(m_renderComponent.get(), &m_device);

		nowCmdList.TransitState(
			m_device.GetSwapChain()->GetCurrentTexture(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT
		);

		CommandList* pCmdLists[] = {&nowCmdList};
		m_device.EndAndExecuteCommandList(pCmdLists, ARRAYSIZE(pCmdLists));
		
		m_device.Present(1);
		
	}
}