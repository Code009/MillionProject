#include "pch.h"

#include "AppModule.h"

using namespace cnLibrary;
using namespace MillionItemDump;

cWinApp::cWinApp()noexcept
{
	MainHWND=cnWindows::CreateWindowHandle(nullptr,L"1234");
	MainWindowClient=cnWindows::CreateWindowClient();

	MainHWND->InsertMessageHandler(&MainWindowMessageHandler);
	MainHWND->SetClient(MainWindowClient);
	MainWindow=iCast<iUIWindow>(MainWindowClient);
}
cWinApp::~cWinApp()noexcept
{
}

void cWinApp::cMainWindowMessageHandler::WindowDetached(void)noexcept(true)
{
	auto Host=cnMemory::GetObjectFromMemberPointer(this,&cWinApp::MainWindowMessageHandler);
	if(Host->OnMainWindowClose!=nullptr)
		Host->OnMainWindowClose();
}


bool cWinApp::cMainWindowMessageHandler::WindowMessage(LRESULT &Result,const cWindowMessageParam &MsgParam)noexcept(true)
{
	if(MsgParam.Code==WM_DESTROY){
		auto Host=cnMemory::GetObjectFromMemberPointer(this,&cWinApp::MainWindowMessageHandler);
		if(Host->OnMainWindowClose!=nullptr)
			Host->OnMainWindowClose();
	}
	return false;
}

void cWinApp::UISessionStart(void)noexcept
{
	auto MainHwnd=MainHWND->GetWindowHandle();
	::ShowWindow(MainHwnd,SW_SHOWDEFAULT);
	UIStarted();
}
void cWinApp::UISessionExit(void)noexcept
{
	UIStopped();
}



cLibModule::cLibModule()
{
	LibraryModule=cnSystem::SystemQueryModule(nullptr);
#ifdef _DEBUG
	cnRTL::gRTLDebuggerContext->DebugStartThread();
#endif // _DEBUG
}
cLibModule::~cLibModule()
{
#ifdef _DEBUG
	cnRTL::gRTLDebuggerContext->DebugShutdownThread();
#endif // _DEBUG
	LibraryModule->CloseAndWaitUnload(nullptr);
	//::CoUninitialize();
}

cSysModule::cSysModule()
{
	UIApplication=cnWindows::CreateWindowsUIApplication();
}
cSysModule::~cSysModule()
{
}


cMillionItemDumpModue::cMillionItemDumpModue()
{
	UIApplication->InsertHandler(&App);

	App.OnMainWindowClose=[this]{
		UIApplication->CloseUISession();
	};
}
cMillionItemDumpModue::~cMillionItemDumpModue()
{
	UIApplication->RemoveHandler(&App);
}


void cMillionItemDumpModue::UIMain(void)
{
	UIApplication->UIMain();
}
