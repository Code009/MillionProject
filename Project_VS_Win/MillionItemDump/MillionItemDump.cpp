#include "AppHeader.h"
#include <chrono>
#include "resource.h"
#include "MillionItemDumpDsk\AppModule.h"

#ifndef cnLibrary_RTL_CORE
#define	cnLibrary_RTL_CORE	"Sc009"


#include <cnRTL/Sc009CoreCommon.h>
#include <cnRTL/Sc009CoreRuntime.h>

#include <cnRTL/Sc009CoreFloat.h>
#include <cnRTL/CoreMath.h>
#include <cnRTL/Sc009CoreLongInteger.h>
#include <cnRTL/CoreStringConvert.h>

#include <cnRTL/Win32CoreThread.h>

#include <cnRTL/Sc009CoreTemplateKit.h>

#endif

#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"d3dcompiler.lib")


using namespace cnLibrary;
using namespace MillionItemDump;

static cnVar::cStaticVariable<cMillionItemDumpModue> _Module;

cApp &MillionItemDump::gApp=_Module->App;
cD3DModule &MillionItemDump::gD3DModule=_Module->D3DModule;


volatile sfInt64 gv;
//---------------------------------------------------------------------------
__declspec(noinline)
static void Test(void){
	//typedef cnVar::cAccumulatedValue<TKRuntime::SystemTime::tTimeOperator> cSystemTime;
	typedef cnVar::cAccumulatedValue<TKRuntime::SystemTime::tRealTimeOperator> cSystemTime;

	cSystemTime t1,t2;
	t1=TKRuntime::SystemTime::GetRealTimeNow();
	//t1=TKRuntime::SystemTime::GetTimeNow();
	::Sleep(1000);
	t2=TKRuntime::SystemTime::GetRealTimeNow();
	//t2=TKRuntime::SystemTime::GetTimeNow();
	//t2+=Time_1s;

	sfInt64 d=t2-t1;


	t1+=d;

	gv=d;

}

static void MPDMain(void)
{
	_Module.Construct();

	Test();
	Test();
	
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

