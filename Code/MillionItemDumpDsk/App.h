#pragma once
#include "AppHeader.h"

#include "TestD12.h"

namespace MillionItemDump{


class cTestD3DPainter : public iDXGIPainter
{
public:
	cTestD3DPainter();
	~cTestD3DPainter();

	void SetView(iUIView *View);

	virtual void cnLib_FUNC PaintStarted(void)override;
	virtual void cnLib_FUNC PaintShow(void)override;
	virtual void cnLib_FUNC PaintResume(void)override;
	virtual void cnLib_FUNC PaintPaused(void)override;
	virtual void cnLib_FUNC PaintHide(void)override;
	virtual void cnLib_FUNC PaintStopped(void)override;

	virtual void cnLib_FUNC PaintSizeChanged(void)override;
	virtual void cnLib_FUNC RenderBufferChanged(void)override;

	void ScaleUp(void);
	void ScaleDown(void);
protected:
	rPtr<iDXGIViewContent> fViewContent;

	COMPtr<ID3D12Device> fDevice;
	COMPtr<ID3D12CommandQueue> fCommandQueue;
	COMPtr<ID3D12Fence> fFence;

	cD12ResourceRenderTarget fSharedRenderTarget;
	cTestRenderCommand fRenderCommand;

	void Render(void);
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
	cMainForm();
	~cMainForm();


	cTestD3DPainter Painter;
	cnUI::cTextButton ScaleUpButton;
	cnUI::cTextButton ScaleDownButton;

protected:
	virtual void cnLib_FUNC UILayout(void)override;
};

class cApp
{
public:
	cApp();
	~cApp();

	iPtr<iUIWindow> MainWindow;

	cMainForm MainForm;

	void UIStarted(void);
	void UIStopped(void);
};


extern cApp &gApp;

}
