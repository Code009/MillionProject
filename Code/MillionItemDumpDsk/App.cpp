#include "pch.h"

#include "App.h"

using namespace cnLibrary;
using namespace cnRTL;
using namespace MillionItemDump;



cTestD3DPainter::cTestD3DPainter()noexcept
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
cTestD3DPainter::~cTestD3DPainter()noexcept
{
	fViewContent=nullptr;
}

void cTestD3DPainter::SetView(iUIView *View)noexcept
{
	fViewContent->SetView(View);
}

void cTestD3DPainter::PaintStateChanged(void)noexcept
{
}

void cTestD3DPainter::PaintSizeChanged(void)noexcept
{
}
void cTestD3DPainter::RenderBufferChanged(void)noexcept
{
	Render();
}

void cTestD3DPainter::Render(void)noexcept{
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

void cTestD3DPainter::ScaleUp(void)noexcept
{
	fRenderCommand.Scale+=0.125;

	Render();
}
void cTestD3DPainter::ScaleDown(void)noexcept
{
	if(fRenderCommand.Scale>1.){
		fRenderCommand.Scale-=0.125;
		Render();
	}
}

cMainForm::cMainForm()noexcept
{
	cnUI::ControlCreateView(ScaleUpButton,fView);
	cnUI::ControlCreateView(ScaleDownButton,fView);

	ScaleUpButton.Text=u"+"_cArray;
	ScaleDownButton.Text=u"-"_cArray;

	ScaleUpButton.OnClick=[this]{
		Painter.ScaleUp();
	};
	
	ScaleDownButton.OnClick=[this]{
		Painter.ScaleDown();
	};
	Painter.SetView(fView);
}
cMainForm::~cMainForm()noexcept
{
}

void cMainForm::UILayout(void)noexcept
{
	cnUI::cLayout Layout;
	Layout.ResetClient(fView);

	{
		auto LineLayout=Layout.LayoutTop(50);

		cnUI::ControlSetRect(ScaleUpButton,fView,LineLayout.LayoutLeft(100,10,10));
		cnUI::ControlSetRect(ScaleDownButton,fView,LineLayout.LayoutLeft(100,10,10));
	}
}


cApp::cApp()noexcept
{
}

cApp::~cApp()noexcept
{
}

void cApp::UIStarted(void)noexcept
{
	MainWindow->SetClient(MainForm);
}
void cApp::UIStopped(void)noexcept
{
	MainWindow->SetClient(nullptr);
}
