// D3DNativeExample.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "AppHeader.h"
#include "resource.h"

#include "MillionItemDumpDsk\App.h"

#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"d3dcompiler.lib")

using namespace cnLibrary;
using namespace cnRTL;

struct cD12Presentation
{
	COMPtr<ID3D12Device> Device;
	COMPtr<ID3D12CommandQueue> CommandQueue;
    COMPtr<IDXGISwapChain> SwapChain;
    COMPtr<IDXGISwapChain3> SwapChain3;
	COMPtr<ID3D12DescriptorHeap> RTVHeap;
	uIntn RTVDescSize;
	COMPtr<ID3D12Resource> RenderTarget[3];

    HANDLE FenceEvent;
    COMPtr<ID3D12Fence> Fence;

	uInt64 FenceValue=1;
	int FrameIndex=0;
	void WaitForPreviousFrame()
	{
		// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
		// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
		// sample illustrates how to use fences for efficient resource usage and to
		// maximize GPU utilization.

		// Signal and increment the fence value.
		UINT64 fence = FenceValue++;
		//CommandQueue->Signal(Fence, fence);
			

		//fence = FenceValue++;
		CommandQueue->Signal(Fence, FenceValue++);

		Fence->SetEventOnCompletion(fence, FenceEvent);

		// Wait until the previous frame is finished.
		//UINT64 cv=Fence->GetCompletedValue();
		//if (cv < fence)
		//{
			WaitForSingleObject(FenceEvent, INFINITE);
		//}

		FrameIndex = SwapChain3->GetCurrentBackBufferIndex();
	}
};

struct cD12Device
{
	cD12Device(){
		CreateDXGIFactory1(__uuidof(IDXGIFactory1),COMRetPtr(DXGIFactory));


		COMPtr<IDXGIAdapter> Adapter;
		DXGIFactory->EnumAdapters(0,COMRetPtrT(Adapter));
		::D3D12CreateDevice(Adapter,D3D_FEATURE_LEVEL_12_0,__uuidof(ID3D12Device),COMRetPtr(Device));
	}
	
	COMPtr<IDXGIFactory1> DXGIFactory;
	COMPtr<IDXGIAdapter> Adapter;

	COMPtr<ID3D12Device> Device;

	void CreateSwapChain(cD12Presentation *Presentation,HWND WindowHandle){
		Presentation->Device=Device;
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		Device->CreateCommandQueue(&queueDesc,__uuidof(ID3D12CommandQueue),COMRetPtr(Presentation->CommandQueue));

		// Describe and create the swap chain.
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferCount = 2;
		swapChainDesc.BufferDesc.Width = 0;
		swapChainDesc.BufferDesc.Height = 0;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.OutputWindow = WindowHandle;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.Windowed = TRUE;

		DXGIFactory->CreateSwapChain(Presentation->CommandQueue,&swapChainDesc,COMRetPtrT(Presentation->SwapChain));
		Presentation->SwapChain->QueryInterface(COMRetPtrT(Presentation->SwapChain3));
	
	
		Device->CreateFence(0, D3D12_FENCE_FLAG_NONE,__uuidof(ID3D12Fence),COMRetPtr(Presentation->Fence));

        // Create an event handle to use for frame synchronization.
        Presentation->FenceEvent = ::CreateEventW(nullptr, FALSE, FALSE, nullptr);

		Presentation->WaitForPreviousFrame();
	}
};


static const char hlsl_Src[]=R"*(
//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PSInput VSMain(float4 position : POSITION, float4 color : COLOR)
{
    PSInput result;

    result.position = position;
    result.color = color;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}
)*";

struct cD12Render
{
	aClsRef<cD12Presentation> Presentation;
	cD12Render(aClsRef<cD12Presentation> pPresentation)
		: Presentation(cnVar::MoveCast(pPresentation))
	{

		D3D12_ROOT_SIGNATURE_DESC rs;
		rs.Flags=D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		rs.NumParameters=0;
		rs.pParameters=nullptr;
		rs.NumStaticSamplers=0;
		rs.pStaticSamplers=nullptr;

		COMPtr<ID3DBlob> b;
		COMPtr<ID3DBlob> eb;
		D3D12SerializeRootSignature(&rs,D3D_ROOT_SIGNATURE_VERSION_1,COMRetPtrT(b),COMRetPtrT(eb));
		Presentation->Device->CreateRootSignature(0,b->GetBufferPointer(),b->GetBufferSize(),__uuidof(ID3D12RootSignature),COMRetPtr(RootSign));

#if defined(_DEBUG)
        // Enable better shader debugging with the graphics debugging tools.
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif
        COMPtr<ID3DBlob> VertexShader;
        COMPtr<ID3DBlob> PixelShader;

        //ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
		D3DCompile(hlsl_Src,sizeof(hlsl_Src)-1,"VS",nullptr,nullptr,"VSMain", "vs_5_0", compileFlags, 0, COMRetPtrT(VertexShader), nullptr);
		D3DCompile(hlsl_Src,sizeof(hlsl_Src)-1,"PS",nullptr,nullptr,"PSMain", "ps_5_0", compileFlags, 0, COMRetPtrT(PixelShader), nullptr);

        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.pRootSignature = RootSign;
		psoDesc.InputLayout.NumElements=cnMemory::ArrayLength(inputElementDescs);
        psoDesc.InputLayout.pInputElementDescs=inputElementDescs;
        psoDesc.VS = { reinterpret_cast<UINT8*>(VertexShader->GetBufferPointer()), VertexShader->GetBufferSize() };
        psoDesc.PS = { reinterpret_cast<UINT8*>(PixelShader->GetBufferPointer()), PixelShader->GetBufferSize() };
		psoDesc.RasterizerState={};
		psoDesc.RasterizerState.FillMode=D3D12_FILL_MODE_SOLID;
		psoDesc.RasterizerState.CullMode=D3D12_CULL_MODE_BACK;
		psoDesc.RasterizerState.DepthClipEnable=true;
		psoDesc.RasterizerState.ConservativeRaster=D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		psoDesc.BlendState = {};
		psoDesc.BlendState.RenderTarget[0].BlendEnable=false;
		psoDesc.BlendState.RenderTarget[0].LogicOpEnable=false;
		psoDesc.BlendState.RenderTarget[0].SrcBlend=D3D12_BLEND_ONE;
		psoDesc.BlendState.RenderTarget[0].DestBlend=D3D12_BLEND_ZERO;
		psoDesc.BlendState.RenderTarget[0].BlendOp=D3D12_BLEND_OP_ADD;
		psoDesc.BlendState.RenderTarget[0].SrcBlendAlpha=D3D12_BLEND_ONE;
		psoDesc.BlendState.RenderTarget[0].DestBlendAlpha=D3D12_BLEND_ZERO;
		psoDesc.BlendState.RenderTarget[0].BlendOpAlpha=D3D12_BLEND_OP_ADD;
		psoDesc.BlendState.RenderTarget[0].LogicOp=D3D12_LOGIC_OP_NOOP;
		psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask=D3D12_COLOR_WRITE_ENABLE_ALL;
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
		Presentation->Device->CreateGraphicsPipelineState(&psoDesc,__uuidof(ID3D12PipelineState),COMRetPtr(PState));




		Presentation->Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,__uuidof(ID3D12CommandAllocator),COMRetPtr(CmdAllocator));
		
		// Create the command list.
		Presentation->Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CmdAllocator, PState, __uuidof(ID3D12GraphicsCommandList),COMRetPtr(CmdList));

		CmdList->Close();
	    // Create the vertex buffer.
		// 
        // Define the geometry for a triangle.
        Vertex triangleVertices[] =
        {
            { { 0.0f, 0.25f , 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
            { { 0.25f, -0.25f , 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
            { { -0.25f, -0.25f , 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
        };

        const UINT vertexBufferSize = sizeof(triangleVertices);

		D3D12_HEAP_PROPERTIES vb_hp={};
		vb_hp.Type=D3D12_HEAP_TYPE_UPLOAD;
		vb_hp.CPUPageProperty=D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		vb_hp.CreationNodeMask=0;
		vb_hp.MemoryPoolPreference=D3D12_MEMORY_POOL_UNKNOWN;
		vb_hp.VisibleNodeMask=0;

		D3D12_RESOURCE_DESC vb_rd={};
		vb_rd.Dimension=D3D12_RESOURCE_DIMENSION_BUFFER;
		vb_rd.Alignment=D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		vb_rd.DepthOrArraySize=1;
		vb_rd.Width=vertexBufferSize;
		//vb_rd.Width=65536;
		vb_rd.Height=1;
		vb_rd.MipLevels=1;
		vb_rd.Format=DXGI_FORMAT_UNKNOWN;
		vb_rd.SampleDesc.Count=1;
		vb_rd.SampleDesc.Quality=0;
		vb_rd.Layout=D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		vb_rd.Flags=D3D12_RESOURCE_FLAG_NONE;
		
		HRESULT hr;

        // Note: using upload heaps to transfer static data like vert buffers is not 
        // recommended. Every time the GPU needs it, the upload heap will be marshalled 
        // over. Please read up on Default Heap usage. An upload heap is used here for 
        // code simplicity and because there are very few verts to actually transfer.
        hr=Presentation->Device->CreateCommittedResource(
            &vb_hp,
            D3D12_HEAP_FLAG_NONE,
            &vb_rd,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
			__uuidof(ID3D12Resource),
            COMRetPtr(VertexBuffer));

        // Copy the triangle data to the vertex buffer.
        UINT8* pVertexDataBegin;
        D3D12_RANGE readRange;        // We do not intend to read from this resource on the CPU.
		readRange.Begin=0;
		readRange.End=0;
        VertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
        memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
        VertexBuffer->Unmap(0, nullptr);

        // Initialize the vertex buffer view.
        VBView.BufferLocation = VertexBuffer->GetGPUVirtualAddress();
        VBView.StrideInBytes = sizeof(Vertex);
        VBView.SizeInBytes = vertexBufferSize;


        // Wait for the command list to execute; we are reusing the same command 
        // list in our main loop but for now, we just want to wait for setup to 
        // complete before continuing.
        //WaitForPreviousFrame();

		
    // Create descriptor heaps.
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = 2;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        Presentation->Device->CreateDescriptorHeap(&rtvHeapDesc, __uuidof(ID3D12DescriptorHeap),COMRetPtr(Presentation->RTVHeap));

        Presentation->RTVDescSize = Presentation->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle=Presentation->RTVHeap->GetCPUDescriptorHandleForHeapStart();
        // Create a RTV for each frame.
        for (UINT n = 0; n < 2; n++)
        {
            Presentation->SwapChain->GetBuffer(n, __uuidof(ID3D12Resource),COMRetPtr(Presentation->RenderTarget[n]));
            Presentation->Device->CreateRenderTargetView(Presentation->RenderTarget[n], nullptr, rtvHandle);
			rtvHandle.ptr+=Presentation->RTVDescSize;
        }


	}
	~cD12Render(){
	}

	COMPtr<ID3D12RootSignature> RootSign;
	COMPtr<ID3D12PipelineState> PState;
	COMPtr<ID3D12CommandAllocator> CmdAllocator;
	COMPtr<ID3D12GraphicsCommandList> CmdList;

	
    struct Vertex
    {
		float position[3];
		float color[4];
    };
	COMPtr<ID3D12Resource> VertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW VBView;

	D3D12_VIEWPORT Viewport;
	D3D12_RECT ScissorRect;

	void UpdateViewport(int x,int y){
		Viewport.TopLeftX=0;
		Viewport.TopLeftY=0;
		Viewport.Width=x;
		Viewport.Height=y;
		Viewport.MinDepth=0.;
		Viewport.MaxDepth=1.;

		ScissorRect.left=0;
		ScissorRect.top=0;
		ScissorRect.right=x;
		ScissorRect.bottom=y;

	}


	void Render(void){
		// Command list allocators can only be reset when the associated 
		// command lists have finished execution on the GPU; apps should use 
		// fences to determine GPU execution progress.
		CmdAllocator->Reset();

		// However, when ExecuteCommandList() is called on a particular command 
		// list, that command list can then be reset at any time and must be before 
		// re-recording.
		CmdList->Reset(CmdAllocator, PState);

		// Set necessary state.
		CmdList->SetGraphicsRootSignature(RootSign);


		CmdList->RSSetViewports(1, &Viewport);
		CmdList->RSSetScissorRects(1, &ScissorRect);

		int FrameIndex=Presentation->FrameIndex;

		D3D12_RESOURCE_BARRIER rb;
		rb.Type=D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		rb.Flags=D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
		rb.Transition.pResource=Presentation->RenderTarget[FrameIndex];
		rb.Transition.Subresource=D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		rb.Transition.StateBefore=D3D12_RESOURCE_STATE_PRESENT;
		rb.Transition.StateAfter=D3D12_RESOURCE_STATE_RENDER_TARGET;
		// Indicate that the back buffer will be used as a render target.
		CmdList->ResourceBarrier(1, &rb);

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle=Presentation->RTVHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr+=FrameIndex*Presentation->RTVDescSize;
		CmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

		// Record commands.
		float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		if(FrameIndex){
			clearColor[0]=1.f;
		}
		
		CmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		CmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		CmdList->IASetVertexBuffers(0, 1, &VBView);
		CmdList->DrawInstanced(3, 1, 0, 0);

		rb.Type=D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		rb.Flags=D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
		rb.Transition.pResource=Presentation->RenderTarget[FrameIndex];
		rb.Transition.Subresource=D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		rb.Transition.StateBefore=D3D12_RESOURCE_STATE_RENDER_TARGET;
		rb.Transition.StateAfter=D3D12_RESOURCE_STATE_PRESENT;
		// Indicate that the back buffer will now be used to present.
		CmdList->ResourceBarrier(1, &rb);

		CmdList->Close();
	
		ID3D12CommandList* ppCommandLists[] = { CmdList };
		Presentation->CommandQueue->ExecuteCommandLists(1, ppCommandLists);

	}


};


extern "C" IMAGE_DOS_HEADER __ImageBase;
static const HINSTANCE gInstance=reinterpret_cast<HINSTANCE>(&__ImageBase);

struct cLibModule
{
	cLibModule(){
		cnWindows::Initialize();
	}
	~cLibModule(){
		cnWindows::Finalize();
	}
};


struct D3DNativeExampleModule : cLibModule
{
	D3DNativeExampleModule(){
		WndClass=MyRegisterClass();

		::LoadString(gInstance,IDS_APP_TITLE,szTitle,cnMemory::ArrayLength(szTitle));

	}

	~D3DNativeExampleModule(){
	}

	cD12Device D12Device;


	ATOM WndClass;
	HWND hWnd;
	COMPtr<IDXGIDevice> TargetDXGI;
	COMPtr<IDXGISwapChain> TargetSwapChain;

	ATOM MyRegisterClass(void)
	{
		WNDCLASSEXW wcex;

		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style          = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc    = WndProc;
		wcex.cbClsExtra     = 0;
		wcex.cbWndExtra     = 0;
		wcex.hInstance      = gInstance;
		wcex.hIcon          = LoadIcon(gInstance, MAKEINTRESOURCE(IDI_D3DNATIVEEXAMPLE));
		wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
		wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_D3DNATIVEEXAMPLE);
		wcex.lpszClassName  = L"D3DNativeExample";
		wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

		return RegisterClassExW(&wcex);
	}
	
	WCHAR szTitle[128];                  // The title bar text

	void UIMain(int nCmdShow){

		hWnd = CreateWindowW((LPCWSTR)WndClass, szTitle, WS_OVERLAPPEDWINDOW,
		  CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, gInstance, nullptr);

		if (!hWnd)
		{
			return;
		}

		Presentation=aClsCreate<cD12Presentation>();
		D12Device.CreateSwapChain(Presentation,hWnd);

		Render=aClsCreate<cD12Render>(Presentation);

		ShowWindow(hWnd, nCmdShow);
		UpdateWindow(hWnd);

		MSG msg;

		// Main message loop:
		while (GetMessage(&msg, nullptr, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	static LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

	aClsRef<cD12Presentation> Presentation;
	aClsRef<cD12Render> Render;
};
static D3DNativeExampleModule gModule;


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

	gModule.UIMain(nCmdShow);

	return 0;
}


// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT D3DNativeExampleModule::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(gInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
		::ValidateRect(hWnd,nullptr);
            //PAINTSTRUCT ps;
            //HDC hdc = BeginPaint(hWnd, &ps);
            //// TODO: Add any drawing code that uses hdc here...
            //EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
	case WM_SIZE:
		{
			RECT rc;
			::GetClientRect(hWnd,&rc);
			gModule.Render->UpdateViewport(rc.right,rc.bottom);
			//gModule.Presentation->SwapChain->ResizeBuffers(0,rc.right,rc.bottom,DXGI_FORMAT_UNKNOWN,0);
			gModule.Presentation->SwapChain->ResizeBuffers(0,0,0,DXGI_FORMAT_UNKNOWN,0);
		}
		break;
	case WM_LBUTTONDOWN:
	{
		gModule.Render->Render();
		HRESULT hr=gModule.Presentation->SwapChain->Present(1,0);
		gModule.Presentation->WaitForPreviousFrame();
	}
		break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
