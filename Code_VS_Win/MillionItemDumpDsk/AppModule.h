#pragma once
#include <cnUI\WinUI.h>

#include "MillionItemDumpDsk\App.h"

namespace MillionItemDump{

class cWinApp : public cApp, public iWindowsUISessionHandler
{
public:
	cWinApp()noexcept;
	~cWinApp()noexcept;
	iPtr<iWindow> MainHWND;
	iPtr<iWindowClient> MainWindowClient;

	virtual void cnLib_FUNC UISessionStart(void)noexcept(true)override;
	virtual void cnLib_FUNC UISessionExit(void)noexcept(true)override;


	cFunction<void (void)noexcept> OnMainWindowClose;

private:
	class cMainWindowMessageHandler : public cnUI::WindowMessageHandler
	{
	public:
		virtual void cnLib_FUNC WindowDetached(void)noexcept(true)override;
		virtual bool cnLib_FUNC WindowMessage(LRESULT &Result,const cWindowMessageParam &MsgParam)noexcept(true)override;
	}MainWindowMessageHandler;
};

struct cLibModule
{
	cLibModule();
	~cLibModule();
};


struct cSysModule : cLibModule
{
	cSysModule();
	~cSysModule();

	rPtr<iWindowsUIApplication> UIApplication;

};

struct cMillionItemDumpModue : cSysModule
{
	cMillionItemDumpModue();
	~cMillionItemDumpModue();
	
	cD3DModule D3DModule;
	cWinApp App;

	void UIMain(void);
};

}
