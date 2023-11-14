#pragma once

#include "MillionItemDumpDsk\App.h"

namespace MillionItemDump{

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

	iPtr<iWindow> MainWindow;
	iPtr<iWindowClient> MainWindowClient;
};

struct cMillionItemDumpModue : cSysModule
{
	cMillionItemDumpModue();
	~cMillionItemDumpModue();

	cApp App;

	void UIMain(void);
};

}
