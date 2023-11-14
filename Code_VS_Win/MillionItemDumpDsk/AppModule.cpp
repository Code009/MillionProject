#include "pch.h"

#include "AppModule.h"

using namespace cnLibrary;
using namespace MillionItemDump;

cLibModule::cLibModule()
{
	cnWindows::Initialize();
}
cLibModule::~cLibModule()
{
	::CoUninitialize();
}


cSysModule::cSysModule()
{
	UIApplication=cnWindows::CreateWindowsUIApplication();

	MainWindow=cnWindows::CreateHWND(nullptr,L"1234");
	MainWindowClient=cnWindows::CreateWindowClient();

	MainWindowClient->SetWindow(MainWindow);

}
cSysModule::~cSysModule()
{
}


cMillionItemDumpModue::cMillionItemDumpModue()
{
	App.MainWindow=iCast<iUIWindow>(MainWindowClient);
}
cMillionItemDumpModue::~cMillionItemDumpModue()
{
}


void cMillionItemDumpModue::UIMain(void)
{
	App.UIStarted();
	UIApplication->UIMain(MainWindow,SW_SHOW);
	App.UIStopped();
}
