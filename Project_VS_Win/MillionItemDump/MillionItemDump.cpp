#include "AppHeader.h"
#include "resource.h"
#include "MillionItemDumpDsk\AppModule.h"

#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"d3dcompiler.lib")


using namespace cnLibrary;
using namespace MillionItemDump;

static cnVar::cStaticVariable<cMillionItemDumpModue> _Module;

cApp &MillionItemDump::gApp=_Module->App;


static void MPDMain(void)
{
	_Module.Construct();
	
	_Module->UIMain();

	_Module.Destruct();
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	MPDMain();

    return 0;
}

