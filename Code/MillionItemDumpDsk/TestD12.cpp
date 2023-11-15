#include "pch.h"
#include "TestD12.h"


using namespace cnLibrary;
using namespace cnRTL;
using namespace MillionItemDump;

cD12Device::cD12Device()
{
	HRESULT hr;
	D3D12CreateDevice(nullptr,D3D_FEATURE_LEVEL_12_0,__uuidof(ID3D12Device),COMRetPtr(fDevice));
}
cD12Device::~cD12Device()
{
}

cD12Device::operator ID3D12Device *()
{
	return fDevice;
}


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
cTestRenderCommand::cTestRenderCommand(ID3D12Device *Device)
	: fDevice(Device)
{
	HRESULT hr;

	D3D12_ROOT_SIGNATURE_DESC rs;
	rs.Flags=D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rs.NumParameters=0;
	rs.pParameters=nullptr;
	rs.NumStaticSamplers=0;
	rs.pStaticSamplers=nullptr;

	COMPtr<ID3DBlob> b;
	COMPtr<ID3DBlob> eb;
	D3D12SerializeRootSignature(&rs,D3D_ROOT_SIGNATURE_VERSION_1,COMRetPtrT(b),COMRetPtrT(eb));
	fDevice->CreateRootSignature(0,b->GetBufferPointer(),b->GetBufferSize(),__uuidof(ID3D12RootSignature),COMRetPtr(fRootSign));
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
    psoDesc.pRootSignature = fRootSign;
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
	fDevice->CreateGraphicsPipelineState(&psoDesc,__uuidof(ID3D12PipelineState),COMRetPtr(fPState));

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
		

    // Note: using upload heaps to transfer static data like vert buffers is not 
    // recommended. Every time the GPU needs it, the upload heap will be marshalled 
    // over. Please read up on Default Heap usage. An upload heap is used here for 
    // code simplicity and because there are very few verts to actually transfer.
    hr=fDevice->CreateCommittedResource(
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

}
cTestRenderCommand::~cTestRenderCommand()
{
}

void cTestRenderCommand::Build(const cD12RenderTarget &Target)
{
	if(fCmdAllocator==nullptr){
		fDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,__uuidof(ID3D12CommandAllocator),COMRetPtr(fCmdAllocator));
		fDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, fCmdAllocator, fPState, __uuidof(ID3D12GraphicsCommandList),COMRetPtr(fCmdList));
	}
	else{
		fCmdAllocator->Reset();
		fCmdList->Reset(fCmdAllocator, fPState);
	}

	ID3D12GraphicsCommandList *CmdList=fCmdList;
	// Set necessary state.
	CmdList->SetGraphicsRootSignature(fRootSign);

	D3D12_VIEWPORT Viewport;
	D3D12_RECT ScissorRect;

	Viewport.TopLeftX=0;
	Viewport.TopLeftY=0;
	Viewport.Width=Target.Width;
	Viewport.Height=Target.Height;
	Viewport.MinDepth=0.;
	Viewport.MaxDepth=1.;

	ScissorRect.left=0;
	ScissorRect.top=0;
	ScissorRect.right=Target.Width;
	ScissorRect.bottom=Target.Height;

	CmdList->RSSetViewports(1, &Viewport);
	CmdList->RSSetScissorRects(1, &ScissorRect);

	D3D12_RESOURCE_BARRIER rb;
	rb.Type=D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	rb.Flags=D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
	rb.Transition.pResource=Target.Resource;
	rb.Transition.Subresource=D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	rb.Transition.StateBefore=D3D12_RESOURCE_STATE_PRESENT;
	rb.Transition.StateAfter=D3D12_RESOURCE_STATE_RENDER_TARGET;
	// Indicate that the back buffer will be used as a render target.
	CmdList->ResourceBarrier(1, &rb);

	CmdList->OMSetRenderTargets(1, &Target.Descriptor, FALSE, nullptr);

	// Record commands.
	float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };

		
	CmdList->ClearRenderTargetView(Target.Descriptor, clearColor, 0, nullptr);
	CmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	CmdList->IASetVertexBuffers(0, 1, &VBView);
	CmdList->DrawInstanced(3, 1, 0, 0);

	rb.Type=D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	rb.Flags=D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
	rb.Transition.pResource=Target.Resource;
	rb.Transition.Subresource=D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	rb.Transition.StateBefore=D3D12_RESOURCE_STATE_RENDER_TARGET;
	rb.Transition.StateAfter=D3D12_RESOURCE_STATE_PRESENT;
	// Indicate that the back buffer will now be used to present.
	CmdList->ResourceBarrier(1, &rb);

	CmdList->Close();
}
	
ID3D12CommandList* cTestRenderCommand::GetCommandList(void)
{
	return fCmdList;
}

