#pragma once
#include "AppHeader.h"


namespace MillionItemDump{


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





}