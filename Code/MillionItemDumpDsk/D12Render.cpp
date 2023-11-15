#include "pch.h"

#include "D12Render.h"

using namespace cnLibrary;
using namespace cnRTL;
using namespace MillionItemDump;


cD12FenceWaiter::cD12FenceWaiter()
{
	fFenceEvent=::CreateEvent(nullptr,TRUE,TRUE,nullptr);
}
cD12FenceWaiter::~cD12FenceWaiter()
{
	::CloseHandle(fFenceEvent);
}

void cD12FenceWaiter::Setup(ID3D12Fence *Fence,UINT64 WaitValue)
{
	auto CurValue=Fence->GetCompletedValue();
	if(CurValue>=WaitValue){
		return;
	}
	::ResetEvent(fFenceEvent);
	Fence->SetEventOnCompletion(WaitValue,fFenceEvent);
}
void cD12FenceWaiter::Wait(void)
{
	WaitForSingleObject(fFenceEvent, INFINITE);
}
void cD12FenceWaiter::Wait(DWORD MillionSeconds)
{
	WaitForSingleObject(fFenceEvent, MillionSeconds);
}


cD12FenceAsyncWaiter::cD12FenceAsyncWaiter()
{
	fFenceEvent=::CreateEvent(nullptr,FALSE,FALSE,nullptr);
}
cD12FenceAsyncWaiter::~cD12FenceAsyncWaiter()
{
	::CloseHandle(fFenceEvent);
}

void cD12FenceAsyncWaiter::VirtualStarted(void)
{
	cDualReference::VirtualStarted();
	fWaiter=cnWindows::DefaultThreadPool->CreateHandleWaiter(&fInnerReference,&fHandleWaitProcedure);
}
void cD12FenceAsyncWaiter::VirtualStopped(void)
{
	fWaiter->SetWait(nullptr,nullptr);
	fWaiter=nullptr;
	cDualReference::VirtualStopped();
}

void cD12FenceAsyncWaiter::Setup(ID3D12Fence *Fence,UINT64 WaitValue)
{
	auto CurValue=Fence->GetCompletedValue();
	if(CurValue>=WaitValue){
		FenceSignaled();
		return;
	}

	Fence->SetEventOnCompletion(WaitValue,fFenceEvent);
	fWaiter->SetWait(fFenceEvent,nullptr);
}


void cD12FenceAsyncWaiter::cHandleWaitProcedure::Execute(DWORD)
{
	auto Host=cnMemory::GetObjectFromMemberPointer(this,&cD12FenceAsyncWaiter::fHandleWaitProcedure);
	Host->FenceSignaled();
}


cD12FenceWaitTask::cD12FenceWaitTask()
{
}
cD12FenceWaitTask::~cD12FenceWaitTask()
{
}

bool cD12FenceWaitTask::IsDone(void)
{
	return fTaskState.IsDone();
}
bool cD12FenceWaitTask::SetNotify(iProcedure *NotifyProcedure)
{
	return fTaskState.SetNotify(NotifyProcedure);
}

void cD12FenceWaitTask::FenceSignaled(void)
{
	fTaskState.SetDone();
}



cD12ResourceRenderTarget::cD12ResourceRenderTarget(ID3D12Device *Device)
	: fDevice(Device)
{
}
cD12ResourceRenderTarget::~cD12ResourceRenderTarget()
{
}

void cD12ResourceRenderTarget::SetupShared(HANDLE RenderBufferHandle,int x,int y)
{
	fRenderTarget=nullptr;
	fDevice->OpenSharedHandle(RenderBufferHandle,__uuidof(ID3D12Resource),COMRetPtr(fRenderTarget));

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 1;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    fDevice->CreateDescriptorHeap(&rtvHeapDesc, __uuidof(ID3D12DescriptorHeap),COMRetPtr(fRenderTargetRTVDesc));

    //fRTVDescSize = fDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle=fRenderTargetRTVDesc->GetCPUDescriptorHandleForHeapStart();
	fDevice->CreateRenderTargetView(fRenderTarget, nullptr, rtvHandle);


	fRenderWidth=x;
	fRenderHeight=y;
}

cD12RenderTarget cD12ResourceRenderTarget::GetRenderTarget(void)
{
	cD12RenderTarget Target;
	Target.Resource=fRenderTarget;
	Target.Descriptor=fRenderTargetRTVDesc->GetCPUDescriptorHandleForHeapStart();
	Target.Width=fRenderWidth;
	Target.Height=fRenderHeight;
	return Target;
}


cD12SwapChainRenderTargets::cD12SwapChainRenderTargets(ID3D12Device *Device)
	: fDevice(Device)
{
}
cD12SwapChainRenderTargets::~cD12SwapChainRenderTargets()
{
}


void cD12SwapChainRenderTargets::SetupHWND(IDXGIFactory2 *DXGIFactory,ID3D12CommandQueue *CommandQueue,HWND WindowHandle)
{
	HRESULT hr;
	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = 2;
	swapChainDesc.Width = 0;
	swapChainDesc.Height = 0;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	swapChainDesc.Flags = 0;

	hr=DXGIFactory->CreateSwapChainForHwnd(CommandQueue,WindowHandle,&swapChainDesc,nullptr,nullptr,COMRetPtrT(fSwapChain));
	hr=fSwapChain->QueryInterface(COMRetPtrT(fSwapChain3));

    // Describe and create a render target view (RTV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 2;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    fDevice->CreateDescriptorHeap(&rtvHeapDesc, __uuidof(ID3D12DescriptorHeap),COMRetPtr(fRenderTargetRTVDesc));

    fRTVDescSize = fDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle=fRenderTargetRTVDesc->GetCPUDescriptorHandleForHeapStart();
    // Create a RTV for each frame.
    for (UINT n = 0; n < 2; n++)
    {
        fSwapChain->GetBuffer(n, __uuidof(ID3D12Resource),COMRetPtr(fRenderTarget[n]));
        fDevice->CreateRenderTargetView(fRenderTarget[n], nullptr, rtvHandle);
		rtvHandle.ptr+=fRTVDescSize;
    }


}


void cD12SwapChainRenderTargets::UpdateViewport(int w,int h)
{
	fRenderWidth=w;
	fRenderHeight=h;
		
	fSwapChain->ResizeBuffers(0,0,0,DXGI_FORMAT_UNKNOWN,0);
}

cD12RenderTarget cD12SwapChainRenderTargets::GetRenderTarget(void)
{
	cD12RenderTarget Target;
	Target.Resource=fRenderTarget[fRenderTargetIndex];
	Target.Descriptor=fRenderTargetRTVDesc->GetCPUDescriptorHandleForHeapStart();
	Target.Descriptor.ptr+=fRTVDescSize*fRenderTargetIndex;
	Target.Width=fRenderWidth;
	Target.Height=fRenderHeight;
	return Target;
}

void cD12SwapChainRenderTargets::Present(void)
{
	fRenderTargetIndex++;
	if(fRenderTargetIndex>=2)
		fRenderTargetIndex=0;
	HRESULT hr=fSwapChain->Present(1,0);
}
