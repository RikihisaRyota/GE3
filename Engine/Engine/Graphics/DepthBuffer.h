#pragma once
/**
 * @file DepthBuffer.h
 * @brief DepthBuffer
 */
#include "PixelBuffer.h"

#include "DescriptorHandle.h"

class DepthBuffer :public PixelBuffer {
public:
	void Create(const std::wstring& name, uint32_t width, uint32_t height, DXGI_FORMAT format);

	float GetClearValue()const { return clearValue_; }
	void SetClearValue(float clearValue) { clearValue_ = clearValue; }

	const DescriptorHandle& GetSRV(void) const { return srvHandle_; }
	const DescriptorHandle& GetDSV(void) const { return dsvHandle_; }
	const DescriptorHandle& GetReadOnlyDSV(void) const { return readOnlyDSVHandle_; }
private:
	void CreateViews();

	float clearValue_ = 1.0f;
	DescriptorHandle dsvHandle_;
	DescriptorHandle readOnlyDSVHandle_;
	DescriptorHandle srvHandle_;
};