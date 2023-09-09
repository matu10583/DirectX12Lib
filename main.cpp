#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif // defined(DEBUG) || defined(_DEBUG)

#include "D3D12FrameWork/TestApp.h"
#include "D3D12FrameWork/MeshShaderApp.h"

int wmain(int argc, wchar_t** argv, wchar_t** envp) {
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(
		_CRTDBG_ALLOC_MEM_DF |
		_CRTDBG_LEAK_CHECK_DF
	);
#endif // defined(DEBUG) || defined(_DEBUG)
	//D3D12FrameWork::TestApp testApp(960, 540);
	//testApp.Run();
	D3D12FrameWork::MeshShaderApp testApp(960, 540);
	testApp.Run();
	return 0;
}