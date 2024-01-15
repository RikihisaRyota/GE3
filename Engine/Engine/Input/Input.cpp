#include "Input.h"

#include <algorithm>
#include <cassert>
#include <string.h>
#include <vector>

#include "Engine/Math/Vector2.h"
#include "../WinApp/WinApp.h"

// デバイス発見時に実行される
BOOL CALLBACK DeviceFindCallBack(LPCDIDEVICEINSTANCE ipddi, LPVOID pvRef) {
	return DIENUM_CONTINUE;
}

Input* Input::GetInstance() {
	static Input instans;
	return &instans;
}

void Input::Initialize() {
	HRESULT result = S_FALSE;
#pragma region DirectInputオブジェクトの生成
	// DirectInputオブジェクトの生成
	result = DirectInput8Create(
		WinApp::GetInstance()->GethInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<void**>(dInput_.GetAddressOf()), nullptr);
	assert(SUCCEEDED(result));
#pragma endregion DirectInputオブジェクトの生成

#pragma region キーボード設定
	// キーボードデバイスの生成
	result = dInput_->CreateDevice(GUID_SysKeyboard, &devKeyboard_, NULL);
	assert(SUCCEEDED(result));

	// 入力データ形式のセット(キーボード)
	result = devKeyboard_->SetDataFormat(&c_dfDIKeyboard); // 標準形式
	assert(SUCCEEDED(result));

	// 排他制御レベルのセット
	result =
		devKeyboard_->SetCooperativeLevel(WinApp::GetInstance()->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));
#pragma endregion キーボード設定

#pragma region マウス設定
	// マウスデバイスの生成
	result = dInput_->CreateDevice(GUID_SysMouse, &devMouse_, NULL);
	assert(SUCCEEDED(result));

	// 入力データ形式のセット
	result = devMouse_->SetDataFormat(&c_dfDIMouse); // マウス用のデータ・フォーマットを設定
	assert(SUCCEEDED(result));

	// 排他制御レベルのセット
	result =
		devMouse_->SetCooperativeLevel(WinApp::GetInstance()->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));
#pragma endregion マウス設定
#pragma region ジョイスティック
	/*result = dInput_->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, this, DIEDFL_ATTACHEDONLY);
	assert(SUCCEEDED(result));*/

	// XInput 初期化
	for (DWORD i = 0; i < XUSER_MAX_COUNT; ++i) {
		Joystick joystick;
		joystick.type_ = PadType::XInput;
		joystick.device_ = nullptr; // XInput では DirectInput デバイスは使用しない
		devJoysticks_.push_back(joystick);
	}
#pragma endregion
}

void Input::Update() {
	devKeyboard_->Acquire(); // キーボード動作開始
	devMouse_->Acquire(); // マウス動作開始
	// 前回のキー入力を保存
	keyPre_ = key_;
	mousePre_ = mouse_;

	// キーの入力
	devKeyboard_->GetDeviceState((DWORD)size(key_), key_.data());

	// マウスの入力
	devMouse_->GetDeviceState(sizeof(DIMOUSESTATE), &mouse_);

	// ジョイスティックの状態を更新
	for (DWORD i = 0; i < devJoysticks_.size(); ++i) {
		if (devJoysticks_[i].type_ == PadType::DirectInput) {
			devJoysticks_[i].device_->Acquire();
			devJoysticks_[i].device_->GetDeviceState(sizeof(DIJOYSTATE2), &devJoysticks_[i].state_.directInput_);
		}
		else if (devJoysticks_[i].type_ == PadType::XInput) {
			XINPUT_STATE xInputState;
			ZeroMemory(&xInputState, sizeof(XINPUT_STATE));
			DWORD result = XInputGetState(i, &xInputState);
			if (result == ERROR_SUCCESS) {
				devJoysticks_[i].state_.xInput_ = xInputState;
				// Zero value if thumbsticks are within the dead zone
				if ((devJoysticks_[i].state_.xInput_.Gamepad.sThumbLX <  XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
					devJoysticks_[i].state_.xInput_.Gamepad.sThumbLX  > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) &&
					(devJoysticks_[i].state_.xInput_.Gamepad.sThumbLY  <  XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
						devJoysticks_[i].state_.xInput_.Gamepad.sThumbLY  > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)) {
					devJoysticks_[i].state_.xInput_.Gamepad.sThumbLX = 0;
					devJoysticks_[i].state_.xInput_.Gamepad.sThumbLY = 0;
				}
				if ((devJoysticks_[i].state_.xInput_.Gamepad.sThumbRX <  XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
					devJoysticks_[i].state_.xInput_.Gamepad.sThumbRX  > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) &&
					(devJoysticks_[i].state_.xInput_.Gamepad.sThumbRY  <  XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
						devJoysticks_[i].state_.xInput_.Gamepad.sThumbRY  > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)) {
					devJoysticks_[i].state_.xInput_.Gamepad.sThumbRX = 0;
					devJoysticks_[i].state_.xInput_.Gamepad.sThumbRY = 0;
				}
			}
		}
	}
}

void Input::Finalize() {
	// デバイスやリソースの解放を行う

	   // マウスデバイスの解放
	if (devMouse_) {
		devMouse_->Unacquire();
		devMouse_->Release();
		devMouse_ = nullptr;
	}

	// キーボードデバイスの解放
	if (devKeyboard_) {
		devKeyboard_->Unacquire();
		devKeyboard_->Release();
		devKeyboard_ = nullptr;
	}

	// ジョイスティックデバイスの解放（必要に応じてループで解放）

	for (auto& joystick : devJoysticks_) {
		if (joystick.device_) {
			joystick.device_->Unacquire();
			joystick.device_->Release();
			joystick.device_ = nullptr;
		}
	}

	// DirectInput インターフェースの解放
	if (dInput_) {
		dInput_->Release();
		dInput_ = nullptr;
	}
}

bool Input::PushKey(BYTE keyNumber) const {

	// 0でなければ押している
	if (key_[keyNumber]) {
		return true;
	}

	// 押していない
	return false;
}

bool Input::PushMouse(int32_t keyNumber) const {
	if (mouse_.rgbButtons[keyNumber]) {
		return true;
	}

	// 押していない
	return false;
}

bool Input::TriggerKey(BYTE keyNumber)const {
	// 前回が0で、今回が0でなければトリガー
	if (!keyPre_[keyNumber] && key_[keyNumber]) {
		return true;
	}

	// トリガーでない
	return false;
}

bool Input::TriggerMouse(int32_t keyNumber) const {
	// 前回が0で、今回が0でなければトリガー
	if (!mousePre_.rgbButtons[keyNumber] && mouse_.rgbButtons[keyNumber]) {
		return true;
	}
	// トリガーでない
	return false;
}

bool Input::ExitKey(BYTE keyNumber) const {
	// 前回が0ではなくて、今回が0
	if (keyPre_[keyNumber] && !key_[keyNumber]) {
		return true;
	}
	return false;
}

bool Input::ExitMouse(int32_t keyNumber) const {
	// 前回が0ではなくて、今回が0
	if (mousePre_.rgbButtons[keyNumber] && !mouse_.rgbButtons[keyNumber]) {
		return true;
	}
	return false;
}

int32_t Input::GetWheel() const {
	return static_cast<int32_t>(mouse_.lZ);
}

Vector2 Input::GetMouseMove() const {
	return {(float)mouse_.lX,(float)mouse_.lY};
}

bool Input::GetJoystickState(int32_t stickNo, DIJOYSTATE2& out) const {
	if (stickNo >= 0 && stickNo < static_cast<int32_t>(devJoysticks_.size())) {
		out = devJoysticks_[stickNo].state_.directInput_;
		return true;
	}
	return false;
}

bool Input::GetJoystickStatePrevious(int32_t stickNo, DIJOYSTATE2& out) const {
	if (stickNo >= 0 && stickNo < static_cast<int32_t>(devJoysticks_.size())) {
		out = devJoysticks_[stickNo].statePre_.directInput_;
		return true;
	}
	return false;
}

bool Input::GetJoystickState(int32_t stickNo, XINPUT_STATE& out) const {
	if (stickNo >= 0 && stickNo < static_cast<int32_t>(devJoysticks_.size())) {
		if (devJoysticks_[stickNo].type_ == PadType::XInput) {
			out = devJoysticks_[stickNo].state_.xInput_;
			return true;
		}
		else {
			// ジョイスティックが接続されていない場合
			return false;
		}
	}
	return false;
}


bool Input::GetJoystickStatePrevious(int32_t stickNo, XINPUT_STATE& out) const {
	if (stickNo >= 0 && stickNo < static_cast<int32_t>(devJoysticks_.size())) {
		if (devJoysticks_[stickNo].type_ == PadType::XInput) {
			out = devJoysticks_[stickNo].statePre_.xInput_;
			return true;
		}
	}
	return false;
}
bool Input::IsControllerConnected() const {
	for (DWORD i = 0; i < devJoysticks_.size(); ++i) {
		if (devJoysticks_[i].type_ == PadType::XInput) {
			XINPUT_STATE xInputState;
			ZeroMemory(&xInputState, sizeof(XINPUT_STATE));
			DWORD result = XInputGetState(i, &xInputState);
			if (result == ERROR_SUCCESS) {
				return true;
			}
			return false;
		}
	}
	return false;
}