// D3DNativeExample.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "AppHeader.h"
#include "resource.h"

#include "MillionItemDumpDsk\TestD12.h"

#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"d3dcompiler.lib")

using namespace cnLibrary;
using namespace cnRTL;
using namespace MillionItemDump;


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
	D3DNativeExampleModule()
		: fDevice(D3DDevice)
		, RenderTargets(D3DDevice)
		, RenderCommand(D3DDevice)
	{
		WndClass=MyRegisterClass();

		::LoadString(gInstance,IDS_APP_TITLE,szTitle,cnMemory::ArrayLength(szTitle));
		
		CreateDXGIFactory1(__uuidof(IDXGIFactory2),COMRetPtr(fDXGIFactory));

		fDevice->CreateFence(0,D3D12_FENCE_FLAG_NONE,__uuidof(ID3D12Fence),COMRetPtr(fFence));
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		fDevice->CreateCommandQueue(&queueDesc,__uuidof(ID3D12CommandQueue),COMRetPtr(fCommandQueue));

	}

	~D3DNativeExampleModule(){
	}

	COMPtr<IDXGIFactory2> fDXGIFactory;
	cD12Device D3DDevice;


	COMPtr<ID3D12Device> fDevice;
	COMPtr<ID3D12CommandQueue> fCommandQueue;
	COMPtr<ID3D12Fence> fFence;

	cD12SwapChainRenderTargets RenderTargets;
	cTestRenderCommand RenderCommand;

	ATOM WndClass;
	HWND hWnd;

	void Render(void){
		auto RenderTarget=RenderTargets.GetRenderTarget();

		RenderCommand.Build(RenderTarget);
		auto CmdList=RenderCommand.GetCommandList();

		fFence->Signal(0);
		cD12FenceWaiter FenceWaiter;
		FenceWaiter.Setup(fFence,1);
		ID3D12CommandList *l[1]={CmdList};
		fCommandQueue->ExecuteCommandLists(1,l);	
		fCommandQueue->Signal(fFence,1);

		FenceWaiter.Wait();

		RenderTargets.Present();
		//gModule.Presentation->WaitForPreviousFrame();
	}

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

		RenderTargets.SetupHWND(fDXGIFactory,fCommandQueue,hWnd);

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
			gModule.RenderTargets.UpdateViewport(rc.right,rc.bottom);
		}
		break;
	case WM_LBUTTONDOWN:
	{
		gModule.RenderCommand.Scale+=0.125;
		gModule.Render();
	}
		break;
	case WM_RBUTTONDOWN:
	{

		if(gModule.RenderCommand.Scale>1.){
			gModule.RenderCommand.Scale-=0.125;
		}
		gModule.Render();
	}
		break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
