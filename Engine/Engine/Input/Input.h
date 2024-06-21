#pragma once

#include <Windows.h>
#include <array>
#include <vector>
#include <wrl.h>

#include <XInput.h>
#define DIRECTINPUT_VERSION 0x0800 // DirectInputのバージョン指定
#include <dinput.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "XInput.lib")

struct Vector2;
enum Button {
	Up = XINPUT_GAMEPAD_DPAD_UP,
	Down = XINPUT_GAMEPAD_DPAD_DOWN,
	Left = XINPUT_GAMEPAD_DPAD_LEFT,
	Right = XINPUT_GAMEPAD_DPAD_RIGHT,
	Start = XINPUT_GAMEPAD_START,
	Back = XINPUT_GAMEPAD_BACK,
	LT = XINPUT_GAMEPAD_LEFT_THUMB,
	RT = XINPUT_GAMEPAD_RIGHT_THUMB,
	LS = XINPUT_GAMEPAD_LEFT_SHOULDER,
	RS = XINPUT_GAMEPAD_RIGHT_SHOULDER,
	A = XINPUT_GAMEPAD_A,
	B = XINPUT_GAMEPAD_B,
	X = XINPUT_GAMEPAD_X,
	Y = XINPUT_GAMEPAD_Y
};
class Input {
public:
	enum class PadType {
		DirectInput,
		XInput,
	};

	union State {
		XINPUT_STATE xInput_;
		DIJOYSTATE2 directInput_;
	};

	struct Joystick {
		Microsoft::WRL::ComPtr<IDirectInputDevice8> device_;
		PadType type_;
		State state_;
		State statePre_;
	};

	static Input* GetInstance();

	void Initialize();
	void Update();
	void Finalize();

	bool PushKey(BYTE keyNumber) const;
	bool TriggerKey(BYTE keyNumber) const;
	bool ExitKey(BYTE keyNumber) const;

	bool PushMouse(int32_t buttonNumber) const;
	bool TriggerMouse(int32_t buttonNumber) const;
	bool ExitMouse(int32_t buttonNumber) const;

	bool PushGamepadButton(Button button, int32_t stickNo = 0) const;
	float GetTriggerPushGamepadButton(Button button, int32_t stickNo = 0) const;
	bool TriggerGamepadButton(Button button, int32_t stickNo = 0) const;
	bool ExitGamepadButton(Button button, int32_t stickNo = 0) const;

	int32_t GetWheel() const;
	Vector2 GetMouseMove() const;

	bool GetJoystickState(DIJOYSTATE2& out, int32_t stickNo = 0) const;
	bool GetJoystickStatePrevious(DIJOYSTATE2& out, int32_t stickNo = 0) const;
	bool GetJoystickState(XINPUT_STATE& out, int32_t stickNo = 0) const;
	bool GetJoystickStatePrevious(XINPUT_STATE& out, int32_t stickNo = 0) const;

	Vector2 GetLeftStick(int32_t stickNo = 0);
	Vector2 GetRightStick(int32_t stickNo = 0);

	bool IsControllerConnected() const;
	bool IsWindowActive();
private:
	Input() = default;
	~Input() = default;
	Input(const Input&) = delete;
	const Input& operator=(const Input&) = delete;

	BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* instance, VOID* context);
	BOOL CALLBACK EnumJoystickObjectsCallback(const DIDEVICEOBJECTINSTANCE* instance, VOID* context);

	void AcquireAllDevices();

private:
	Microsoft::WRL::ComPtr<IDirectInput8> dInput_;
	Microsoft::WRL::ComPtr<IDirectInputDevice8> devKeyboard_;
	Microsoft::WRL::ComPtr<IDirectInputDevice8> devMouse_;
	std::vector<Joystick> devJoysticks_;

	DIMOUSESTATE2 mouse_;
	DIMOUSESTATE2 mousePre_;
	std::array<BYTE, 256> key_;
	std::array<BYTE, 256> keyPre_;
};