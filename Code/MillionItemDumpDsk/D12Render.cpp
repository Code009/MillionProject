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

