#pragma once
/**
 * @file ShaderCompiler.h
 * @brief シェーダーコンパイラー
 */
#include <d3d12.h>
#include <string>
#include <wrl.h>
#include <dxcapi.h>

#pragma comment(lib,"dxcompiler.lib")

class ShaderCompiler {
public:
	static void Initialize();

	static IDxcBlob* Compile(const std::wstring& filePath, const wchar_t* profile);
private:
	static Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
	static Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
	static Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;
};