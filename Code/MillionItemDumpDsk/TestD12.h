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


class cTestRenderCommand
{
public:
	cTestRenderCommand(ID3D12Device *Device);
	~cTestRenderCommand();

	void Build(const cD12RenderTarget &Target);

	ID3D12CommandList* GetCommandList(void);

    struct cTransform
    {
		float WorldMatrix[16];
    };

	float Scale;
	

protected:

	COMPtr<ID3D12Device> fDevice;

	COMPtr<ID3D12RootSignature> fRootSign;
	COMPtr<ID3D12PipelineState> fPState;
	COMPtr<ID3D12CommandAllocator> fCmdAllocator;
	COMPtr<ID3D12GraphicsCommandList> fCmdList;

	COMPtr<ID3D12Resource> fTransformBuffer;

	struct Vertex
    {
		float position[3];
		float color[4];
    };
	COMPtr<ID3D12Resource> fVertexBuffer;

	D3D12_VERTEX_BUFFER_VIEW VBView;


};

}
