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

template<class TData>
class iDataID : public iInterface
{
	virtual eiOrdering cnLib_FUNC Compare(iDataID *Dest)noexcept(true)=0;
};

struct cMillionDumpItem
{
	iPtr< iDataID<cMillionDumpItem> > ID;

	cString<uChar16> Name;

	rPtr<iMillionDumpItemEntity> Parent;

	cSeqList< rPtr<iMillionDumpItemEntity> > Dependents;
};

struct cMillionDumpPlan
{
	iPtr< iDataID<cMillionDumpPlan> > ID;

	iPtr< iDataID<cMillionDumpItem> > Target;
	uInt64 Duration;
	uInt64 Time;
};


class iMillionDumpItemDataSet : public iReference
{
public:
	virtual aClsConstRef<cMillionDumpPlan> QueryPlan(iDataID<cMillionDumpPlan> *ID,bool Refresh)=0;
	virtual iPtr< iDataID<cMillionDumpPlan> > Select(int Condition)=0;
};

class iMillionDumpItemDataTransaction : public iReference
{
public:
	virtual aClsConstRef<cMillionDumpPlan> InsertPlan(const cMillionDumpPlan *Plan)=0;
};

class iMillionItemDataView
{
public:
	virtual rPtr<iMillionDumpItemDataSet> QueryDataSet(void)=0;
	//virtual rPtr<iMillionDumpItemDataTransaction> CreateTransaction(void)=0;
};

class cSQLDatabaseSession
{
};

class bcSQLDatabaseDataView
{
};

class cMillionItemDataView : public bcSQLDatabaseDataView
	, public iMillionItemDataView
{
public:

	class cItemDataSet : public iMillionDumpItemDataSet
	{
	public:
		cItemDataSet(cSQLDatabaseSession *Session)noexcept{
		
		}

		virtual aClsConstRef<cMillionDumpPlan> QueryPlan(iDataID<cMillionDumpPlan> *ID,bool Refresh)override{
			auto Data=aClsCreate<cMillionDumpPlan>();
			return Data;
		}
		virtual iPtr< iDataID<cMillionDumpPlan> > Select(int Condition)override{
			return nullptr;
		}
	protected:

	};

	virtual rPtr<iMillionDumpItemDataSet> QueryDataSet(void)override{
	}
};


class cTestD3DPainter : public iDXGIPainter
{
public:
	cTestD3DPainter()noexcept(true);
	~cTestD3DPainter()noexcept(true);

	void SetView(iUIView *View)noexcept(true);

	virtual void cnLib_FUNC PaintStateChanged(void)noexcept(true)override;
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
