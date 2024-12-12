#include "Input.h"

#include <algorithm>
#include <cassert>
#include <debugapi.h>
#include <string.h>
#include <vector>

#include "Engine/Math/Vector2.h"
#include "../WinApp/WinApp.h"

// デバイス発見時に実行される
BOOL CALLBACK DeviceFindCallBack() {
	return DIENUM_CONTINUE;
}

Input* Input::GetInstance() {
	static Input instance;
	return &instance;
}

void Input::Initialize() {
	HRESULT result = S_FALSE;

	// DirectInputオブジェクトの生成
	result = DirectInput8Create(
		WinApp::GetInstance()->GethInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<void**>(dInput_.GetAddressOf()), nullptr);
	assert(SUCCEEDED(result));

	// キーボード設定
	result = dInput_->CreateDevice(GUID_SysKeyboard, &devKeyboard_, NULL);
	assert(SUCCEEDED(result));
	result = devKeyboard_->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));
	result = devKeyboard_->SetCooperativeLevel(WinApp::GetInstance()->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));

	// マウス設定
	result = dInput_->CreateDevice(GUID_SysMouse, &devMouse_, NULL);
	assert(SUCCEEDED(result));
	result = devMouse_->SetDataFormat(&c_dfDIMouse2);
	assert(SUCCEEDED(result));
	result = devMouse_->SetCooperativeLevel(WinApp::GetInstance()->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));

	// XInput 初期化
	for (DWORD i = 0; i < XUSER_MAX_COUNT; ++i) {
		Joystick joystick;
		joystick.type_ = PadType::XInput;
		devJoysticks_.push_back(joystick);
	}
}
void Input::Update() {
	AcquireAllDevices();

	keyPre_ = key_;
	mousePre_ = mouse_;

	// キーの入力
	devKeyboard_->GetDeviceState((DWORD)key_.size(), key_.data());

	// マウスの入力
	devMouse_->GetDeviceState(sizeof(DIMOUSESTATE2), &mouse_);

	// ジョイスティックの状態を更新
	for (size_t i = 0; i < devJoysticks_.size(); ++i) {
		auto& joystick = devJoysticks_[i];
		if (joystick.type_ == PadType::DirectInput) {
			joystick.device_->Acquire();
			joystick.device_->GetDeviceState(sizeof(DIJOYSTATE2), &joystick.state_.directInput_);
		}
		else if (joystick.type_ == PadType::XInput) {
			XINPUT_STATE xInputState;
			ZeroMemory(&xInputState, sizeof(XINPUT_STATE));
			DWORD result = XInputGetState(static_cast<DWORD>(i), &xInputState);
			if (result == ERROR_SUCCESS) {
				joystick.statePre_ = joystick.state_; // 以前の状態を保存
				joystick.state_.xInput_ = xInputState; // 新しい状態を代入

				// Zero value if thumbsticks are within the dead zone
				auto& gamepad = joystick.state_.xInput_.Gamepad;
				if ((gamepad.sThumbLX <  XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && gamepad.sThumbLX > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) &&
					(gamepad.sThumbLY <  XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && gamepad.sThumbLY > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)) {
					gamepad.sThumbLX = 0;
					gamepad.sThumbLY = 0;
				}
				if ((gamepad.sThumbRX <  XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE && gamepad.sThumbRX > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) &&
					(gamepad.sThumbRY <  XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE && gamepad.sThumbRY > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)) {
					gamepad.sThumbRX = 0;
					gamepad.sThumbRY = 0;
				}
			}
		}
	}
}


void Input::Finalize() {
	if (devMouse_) {
		devMouse_->Unacquire();
		devMouse_->Release();
		devMouse_ = nullptr;
	}
	if (devKeyboard_) {
		devKeyboard_->Unacquire();
		devKeyboard_->Release();
		devKeyboard_ = nullptr;
	}
	for (auto& joystick : devJoysticks_) {
		if (joystick.device_) {
			joystick.device_->Unacquire();
			joystick.device_->Release();
			joystick.device_ = nullptr;
		}
	}
	if (dInput_) {
		dInput_->Release();
		dInput_ = nullptr;
	}
}

bool Input::PushKey(BYTE keyNumber) const {
	return key_[keyNumber] != 0;
}

bool Input::TriggerKey(BYTE keyNumber) const {
	return !keyPre_[keyNumber] && key_[keyNumber];
}

bool Input::ExitKey(BYTE keyNumber) const {
	return keyPre_[keyNumber] && !key_[keyNumber];
}

bool Input::PushMouse(int32_t buttonNumber) const {
	return mouse_.rgbButtons[buttonNumber] != 0;
}

bool Input::TriggerMouse(int32_t buttonNumber) const {
	return !mousePre_.rgbButtons[buttonNumber] && mouse_.rgbButtons[buttonNumber];
}

bool Input::ExitMouse(int32_t buttonNumber) const {
	return mousePre_.rgbButtons[buttonNumber] && !mouse_.rgbButtons[buttonNumber];
}

bool Input::PushGamepadButton(Button button, int32_t stickNo) const {
	if (IsControllerConnected() && stickNo >= 0 && stickNo < static_cast<int32_t>(devJoysticks_.size())) {
		if (button == Button::LT) {
			return (devJoysticks_[stickNo].state_.xInput_.Gamepad.bLeftTrigger) != 0;
		}
		else if (button == Button::RT) {
			return (devJoysticks_[stickNo].state_.xInput_.Gamepad.bRightTrigger) != 0;
		}
		else {
			return (devJoysticks_[stickNo].state_.xInput_.Gamepad.wButtons & button) != 0;
		}
	}
	return false;
}

float Input::GetTriggerPushGamepadButton(Button button, int32_t stickNo) const {
	if (IsControllerConnected() && stickNo >= 0 && stickNo < static_cast<int32_t>(devJoysticks_.size())) {
		if (button == Button::LT) {
			return float(devJoysticks_[stickNo].state_.xInput_.Gamepad.bLeftTrigger) / 255.0f;

		}
		else if (button == Button::RT) {
			return float(devJoysticks_[stickNo].state_.xInput_.Gamepad.bRightTrigger) / 255.0f;
		}
	}
	return 0.0f;
}

bool Input::TriggerGamepadButton(Button button, int32_t stickNo) const {
	if (IsControllerConnected() && stickNo >= 0 && stickNo < static_cast<int32_t>(devJoysticks_.size())) {
		bool wasPressed = (devJoysticks_[stickNo].statePre_.xInput_.Gamepad.wButtons & button) != 0;
		bool isPressed = (devJoysticks_[stickNo].state_.xInput_.Gamepad.wButtons & button) != 0;
		return !wasPressed && isPressed;
	}
	return false;
}

bool Input::ExitGamepadButton(Button button, int32_t stickNo) const {
	if (IsControllerConnected() && stickNo >= 0 && stickNo < static_cast<int32_t>(devJoysticks_.size())) {
		bool wasPressed = (devJoysticks_[stickNo].statePre_.xInput_.Gamepad.wButtons & button) != 0;
		bool isPressed = (devJoysticks_[stickNo].state_.xInput_.Gamepad.wButtons & button) != 0;
		return wasPressed && !isPressed;
	}
	return false;
}
int32_t Input::GetWheel() const {
	return mouse_.lZ;
}

Vector2 Input::GetMouseMove() const {
	return Vector2(static_cast<float>(mouse_.lX), static_cast<float>(mouse_.lY));
}

bool Input::GetJoystickState(DIJOYSTATE2& out, int32_t stickNo) const {
	if (stickNo >= 0 && stickNo < static_cast<int32_t>(devJoysticks_.size()) &&
		devJoysticks_[stickNo].type_ == PadType::DirectInput) {
		out = devJoysticks_[stickNo].state_.directInput_;
		return true;
	}
	return false;
}

bool Input::GetJoystickStatePrevious(DIJOYSTATE2& out, int32_t stickNo) const {
	if (stickNo >= 0 && stickNo < static_cast<int32_t>(devJoysticks_.size()) &&
		devJoysticks_[stickNo].type_ == PadType::DirectInput) {
		out = devJoysticks_[stickNo].statePre_.directInput_;
		return true;
	}
	return false;
}

bool Input::GetJoystickState(XINPUT_STATE& out, int32_t stickNo) const {
	if (stickNo >= 0 && stickNo < static_cast<int32_t>(devJoysticks_.size()) &&
		devJoysticks_[stickNo].type_ == PadType::XInput) {
		out = devJoysticks_[stickNo].state_.xInput_;
		return true;
	}
	return false;
}

bool Input::GetJoystickStatePrevious(XINPUT_STATE& out, int32_t stickNo) const {
	if (stickNo >= 0 && stickNo < static_cast<int32_t>(devJoysticks_.size()) &&
		devJoysticks_[stickNo].type_ == PadType::XInput) {
		out = devJoysticks_[stickNo].statePre_.xInput_;
		return true;
	}
	return false;
}

Vector2 Input::GetLeftStick(int32_t stickNo) {
	if (IsControllerConnected() && stickNo >= 0 && stickNo < static_cast<int32_t>(devJoysticks_.size())) {
		// Get raw values of left stick axes
		int16_t rawX = devJoysticks_[stickNo].state_.xInput_.Gamepad.sThumbLX;
		int16_t rawY = devJoysticks_[stickNo].state_.xInput_.Gamepad.sThumbLY;

		// Normalize values to range [-1.0, 1.0]
		float normalizedX = static_cast<float>(rawX) / 32767.0f; // Map from [-32768, 32767] to [-1.0, 1.0]
		float normalizedY = static_cast<float>(rawY) / 32767.0f; // Map from [-32768, 32767] to [-1.0, 1.0]

		return Vector2(normalizedX, normalizedY);
	}

	// Return default (0, 0) if stickNo is out of range
	return Vector2();
}

Vector2 Input::GetRightStick(int32_t stickNo) {
	if (IsControllerConnected() && stickNo >= 0 && stickNo < static_cast<int32_t>(devJoysticks_.size())) {
		// Get raw values of right stick axes
		int16_t rawX = devJoysticks_[stickNo].state_.xInput_.Gamepad.sThumbRX;
		int16_t rawY = devJoysticks_[stickNo].state_.xInput_.Gamepad.sThumbRY;

		// Normalize values to range [-1.0, 1.0]
		float normalizedX = static_cast<float>(rawX) / 32767.0f; // Map from [-32768, 32767] to [-1.0, 1.0]
		float normalizedY = static_cast<float>(rawY) / 32767.0f; // Map from [-32768, 32767] to [-1.0, 1.0]

		return Vector2(normalizedX, normalizedY);
	}

	// Return default (0, 0) if stickNo is out of range
	return Vector2();
}


bool Input::IsControllerConnected() const {
	for (DWORD i = 0; i < XUSER_MAX_COUNT; ++i) {
		XINPUT_STATE xInputState;
		ZeroMemory(&xInputState, sizeof(XINPUT_STATE));
		DWORD result = XInputGetState(i, &xInputState);
		if (result == ERROR_SUCCESS) {
			return true;
		}
	}
	return false;
}

bool Input::IsWindowActive() {
	return GetForegroundWindow() == WinApp::GetInstance()->GetHwnd();
}


void Input::AcquireAllDevices() {
	if (devKeyboard_) {
		devKeyboard_->Acquire();
	}
	if (devMouse_) {
		devMouse_->Acquire();
	}
	for (auto& joystick : devJoysticks_) {
		if (joystick.device_) {
			joystick.device_->Acquire();
		}
	}
}