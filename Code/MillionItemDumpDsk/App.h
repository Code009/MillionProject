#pragma once
#include "AppHeader.h"

#include "TestD12.h"

namespace MillionItemDump{

class iMillionDumpItemEntity;

/*

ProejctItem
	Name
	Parent
	DependentItem	->

Plan
	ProjectItem	->
	Duration
	Date

Execute
	ProjectItem ->
	Duration
	Date

*/



struct cMillionDumpItem
{
	cString<uChar16> Name;

	rPtr<iMillionDumpItemEntity> Parent;

	cSeqList< rPtr<iMillionDumpItemEntity> > Dependents;
};

class iMillionDumpItemEntity : public iInterface
{
public:
	virtual const cMillionDumpItem* GetItem(void)=0;
};

struct cMillionDumpPlan
{
	rPtr<iMillionDumpItemEntity> Target;
	uInt64 Duration;
	uInt64 Time;
};

class iMillionDumpPlanID : public iInterface
{
};
class iMillionDumpPlanEntity : public iInterface
{
public:
	virtual iInterface* GetID(void)=0;
	virtual const cMillionDumpPlan* GetPlan(void)=0;
};

class iMillionDumpItemDataSet : public iReference
{
public:
	virtual const cMillionDumpPlan* QueryPlan(iMillionDumpPlanID *ID,bool Refresh)=0;
	virtual iMillionDumpPlanID* Select(int Condition)=0;
};

class iMillionDumpItemDataTransaction : public iReference
{
public:
	virtual iPtr<iMillionDumpPlanEntity> InsertPlan(const cMillionDumpPlan *Plan)=0;
};

class iMillionItemDataView
{
public:
	virtual rPtr<iMillionDumpItemDataSet> QueryDataSet(void)=0;
	virtual rPtr<iMillionDumpItemDataTransaction> CreateTransaction(void)=0;
};


class cTestD3DPainter : public iDXGIPainter
{
public:
	cTestD3DPainter()noexcept(true);
	~cTestD3DPainter()noexcept(true);

	void SetView(iUIView *View)noexcept(true);

	virtual void cnLib_FUNC PaintStarted(void)noexcept(true)override;
	virtual void cnLib_FUNC PaintShow(void)noexcept(true)override;
	virtual void cnLib_FUNC PaintResume(void)noexcept(true)override;
	virtual void cnLib_FUNC PaintPaused(void)noexcept(true)override;
	virtual void cnLib_FUNC PaintHide(void)noexcept(true)override;
	virtual void cnLib_FUNC PaintStopped(void)noexcept(true)override;

	virtual void cnLib_FUNC PaintSizeChanged(void)noexcept(true)override;
	virtual void cnLib_FUNC RenderBufferChanged(void)noexcept(true)override;

	void ScaleUp(void)noexcept(true);
	void ScaleDown(void)noexcept(true);
protected:
	rPtr<iDXGIViewContent> fViewContent;

	COMPtr<ID3D12Device> fDevice;
	COMPtr<ID3D12CommandQueue> fCommandQueue;
	COMPtr<ID3D12Fence> fFence;

	cD12ResourceRenderTarget fSharedRenderTarget;
	cTestRenderCommand fRenderCommand;

	void Render(void)noexcept;
};


class cD3DModule
{
public:
	cD12Device D3DDevice;
};


extern cD3DModule &gD3DModule;

class cMainForm : public cnUI::Form
{
public:
	cMainForm()noexcept(true);
	~cMainForm()noexcept(true);


	cTestD3DPainter Painter;
	cnUI::cTextButton ScaleUpButton;
	cnUI::cTextButton ScaleDownButton;

protected:
	virtual void cnLib_FUNC UILayout(void)noexcept(true)override;
};

class cApp
{
public:
	cApp()noexcept(true);
	~cApp()noexcept(true);

	iPtr<iUIWindow> MainWindow;

	cMainForm MainForm;

	void UIStarted(void)noexcept(true);
	void UIStopped(void)noexcept(true);
};


extern cApp &gApp;

}
