#include "StructuredBuffer.h"

#include "GraphicsCore.h"

void StructuredBuffer::Create(const std::wstring& name, size_t bufferSize, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc) {
	buffer_.Create(name, bufferSize);
	auto graphics = GraphicsCore::GetInstance();
	srvHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	graphics->GetDevice()->CreateShaderResourceView(
		buffer_,
		&desc,
		srvHandle_
	);
}

void StructuredBuffer::Copy(const void* srcData, size_t copySize) const {
	buffer_.Copy(srcData,copySize);
}

void StructuredBuffer::ResetBuffer() {
	buffer_.ResetBuffer();
}
