#pragma once
#include "AppHeader.h"
#include <dxgi1_4.h>


namespace MillionItemDump{
using namespace cnLibrary;
using namespace cnRTL;

struct cD12RenderTarget
{
	ID3D12Resource *Resource;
	D3D12_CPU_DESCRIPTOR_HANDLE Descriptor;
	int Width;
	int Height;
};


class cD12FenceWaiter
{
public:
	cD12FenceWaiter();
	~cD12FenceWaiter();

	void Setup(ID3D12Fence *Fence,UINT64 WaitValue);
	void Wait(void);
	void Wait(DWORD MillionSeconds);
protected:
	HANDLE fFenceEvent;
};

class cD12FenceAsyncWaiter : public cDualReference
{
public:
	cD12FenceAsyncWaiter();
	~cD12FenceAsyncWaiter();

	void Setup(ID3D12Fence *Fence,UINT64 WaitValue);

protected:

	void VirtualStarted(void);
	void VirtualStopped(void);

	rPtr<iThreadPoolHandleWaiter> fWaiter;
	HANDLE fFenceEvent;
	
	virtual void FenceSignaled(void)=0;

private:
	class cHandleWaitProcedure : public iFunction<void (DWORD)>
	{
		virtual void cnLib_FUNC Execute(DWORD)override;
	}fHandleWaitProcedure;

};

class cD12FenceWaitTask : public iAsyncTask, public cD12FenceAsyncWaiter
{
public:
	cD12FenceWaitTask();
	~cD12FenceWaitTask();

	virtual bool cnLib_FUNC IsDone(void)override;
	virtual bool cnLib_FUNC SetNotify(iProcedure *NotifyProcedure)override;

protected:
	cAsyncTaskState fTaskState;

	virtual void FenceSignaled(void)override;
};




class cD12ResourceRenderTarget
{
public:
	cD12ResourceRenderTarget(ID3D12Device *Device);
	~cD12ResourceRenderTarget();

	void SetupShared(HANDLE RenderBufferHandle,int x,int y);

	cD12RenderTarget GetRenderTarget(void);

protected:

	COMPtr<ID3D12Device> fDevice;

	COMPtr<ID3D12Resource> fRenderTarget;
	COMPtr<ID3D12DescriptorHeap> fRenderTargetRTVDesc;

	int fRenderWidth;
	int fRenderHeight;
};


class cD12SwapChainRenderTargets
{
public:
	cD12SwapChainRenderTargets(ID3D12Device *Device);
	~cD12SwapChainRenderTargets();


	void SetupHWND(IDXGIFactory2 *DXGIFactory,ID3D12CommandQueue *CommandQueue,HWND WindowHandle);
	void UpdateViewport(int w,int h);
	cD12RenderTarget GetRenderTarget(void);

	void Present(void);


protected:

	COMPtr<ID3D12Device> fDevice;

	COMPtr<IDXGISwapChain1> fSwapChain;
	COMPtr<IDXGISwapChain3> fSwapChain3;
	COMPtr<ID3D12DescriptorHeap> fRenderTargetRTVDesc;
	uIntn fRTVDescSize;
	ID3D12Resource *fRenderTarget[2];

	int fRenderWidth;
	int fRenderHeight;

	ufInt8 fRenderTargetIndex=0;
};

}