#pragma once
#include "AppHeader.h"

#include "D12Render.h"

namespace MillionItemDump{


class cD12Device
{
public:
	cD12Device();
	~cD12Device();

	operator ID3D12Device *();
private:
	COMPtr<ID3D12Device> fDevice;
};




class cTestRenderTargets
{
public:
	cTestRenderTargets(ID3D12Device *Device);
	~cTestRenderTargets();

	void Setup(HANDLE RenderBufferHandle,int x,int y);

	cD12RenderTarget GetRenderTarget(void);

protected:

	COMPtr<ID3D12Device> fDevice;

	COMPtr<ID3D12Resource> fRenderTarget;
	COMPtr<ID3D12DescriptorHeap> fRenderTargetRTVDesc;

	int fRenderWidth;
	int fRenderHeight;
};


class cTestRenderCommand
{
public:
	cTestRenderCommand(ID3D12Device *Device);
	~cTestRenderCommand();

	void Build(const cD12RenderTarget &Target);

	ID3D12CommandList* GetCommandList(void);

protected:

	COMPtr<ID3D12Device> fDevice;

	COMPtr<ID3D12RootSignature> fRootSign;
	COMPtr<ID3D12PipelineState> fPState;
	COMPtr<ID3D12CommandAllocator> fCmdAllocator;
	COMPtr<ID3D12GraphicsCommandList> fCmdList;

    struct Vertex
    {
		float position[3];
		float color[4];
    };
	COMPtr<ID3D12Resource> VertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW VBView;


};
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
protected:
	rPtr<iDXGIViewContent> fViewContent;

	COMPtr<ID3D12Device> fDevice;
	COMPtr<ID3D12CommandQueue> fCommandQueue;
	COMPtr<ID3D12Fence> fFence;

	cTestRenderTargets fSharedRenderTarget;
	cTestRenderCommand fRenderCommand;

};

class cMainForm : public cnUI::Form
{
public:
	cMainForm();
	~cMainForm();


	cTestD3DPainter Painter;
	cnUI::cTextButton Button;

protected:
	virtual void cnLib_FUNC UILayout(void)override;
};

class cApp
{
public:
	cApp();
	~cApp();

	cD12Device D3DDevice;

	iPtr<iUIWindow> MainWindow;

	cMainForm MainForm;

	void UIStarted(void);
	void UIStopped(void);
};


extern cApp &gApp;

}
