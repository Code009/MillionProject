#include "pch.h"

#include "App.h"

using namespace cnLibrary;
using namespace cnRTL;
using namespace MillionItemDump;



cTestD3DPainter::cTestD3DPainter()
	: fDevice(gD3DModule.D3DDevice)
	, fSharedRenderTarget(gD3DModule.D3DDevice)
	, fRenderCommand(gD3DModule.D3DDevice)
{
	HRESULT hr;
	fViewContent=cnWindows::CreateDXGIViewContent(this);

	fDevice->CreateFence(0,D3D12_FENCE_FLAG_NONE,__uuidof(ID3D12Fence),COMRetPtr(fFence));
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	fDevice->CreateCommandQueue(&queueDesc,__uuidof(ID3D12CommandQueue),COMRetPtr(fCommandQueue));

    // Wait for the command list to execute; we are reusing the same command 
    // list in our main loop but for now, we just want to wait for setup to 
    // complete before continuing.
    //WaitForPreviousFrame();
}
cTestD3DPainter::~cTestD3DPainter()
{
	fViewContent=nullptr;
}

void cTestD3DPainter::SetView(iUIView *View)
{
	fViewContent->SetView(View);
}

void cTestD3DPainter::PaintStarted(void)
{
}
void cTestD3DPainter::PaintShow(void)
{
}
void cTestD3DPainter::PaintResume(void)
{
}
void cTestD3DPainter::PaintPaused(void)
{
}
void cTestD3DPainter::PaintHide(void)
{
}
void cTestD3DPainter::PaintStopped(void)
{
}

void cTestD3DPainter::PaintSizeChanged(void)
{
}
void cTestD3DPainter::RenderBufferChanged(void)
{
	HANDLE RenderBufferHandle;
	fViewContent->GetRenderBufferSharedHandle(RenderBufferHandle);

	auto Size=fViewContent->GetPaintSize();
	fSharedRenderTarget.SetupShared(RenderBufferHandle,static_cast<int>(Size.x),static_cast<int>(Size.y));

	auto RenderTarget=fSharedRenderTarget.GetRenderTarget();

	fRenderCommand.Build(RenderTarget);

	auto CmdList=fRenderCommand.GetCommandList();


	fFence->Signal(0);
	cD12FenceWaiter FenceWaiter;

	FenceWaiter.Setup(fFence,1);

	ID3D12CommandList *l[1]={CmdList};
	fCommandQueue->ExecuteCommandLists(1,l);
	
	fCommandQueue->Signal(fFence,1);

	FenceWaiter.Wait();

	fViewContent->UpdateRenderBuffer();
}

cMainForm::cMainForm()
{
	cnUI::ControlCreateView(Button,fView);

	Button.Text=u"1234"_cArray;

	Button.OnClick=[this]{
		::OutputDebugStringW(L"1234\n");
	};

	Painter.SetView(fView);
}
cMainForm::~cMainForm()
{
}

void cMainForm::UILayout(void)
{
	cnUI::cLayout Layout;
	Layout.ResetClient(fView);

	{
		auto LineLayout=Layout.LayoutTop(50);

		cnUI::ControlSetRect(Button,fView,LineLayout.LayoutLeft(100));
	}
}


cApp::cApp()
{
}

cApp::~cApp()
{
}

void cApp::UIStarted(void)
{
	MainWindow->SetClient(MainForm);
}
void cApp::UIStopped(void)
{
	MainWindow->SetClient(nullptr);
}
