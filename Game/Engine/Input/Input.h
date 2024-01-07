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
class Input {
public:
	enum class PadType {
		DirectInput,
		XInput,
	};

	// variantがC++17から
	union State {
		XINPUT_STATE xInput_;
		DIJOYSTATE2 directInput_;
	};
	struct Joystick {
		Microsoft::WRL::ComPtr<IDirectInputDevice8> device_;
		int32_t deadZoneL_;
		int32_t deadZoneR_;
		PadType type_;
		State state_;
		State statePre_;
	};
public: // 静的メンバ関数
	static Input* GetInstance();
public: // メンバ関数
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 毎フレーム処理
	/// </summary>
	void Update();

	/// <summary>
	/// 解放
	/// </summary>
	void Finalize();

	/// <summary>
	/// キーの押下をチェック
	/// </summary>
	/// <param name="keyNumber">キー番号( DIK_0 等)</param>
	/// <returns>押されているか</returns>
	bool PushKey(BYTE keyNumber) const;

	/// <summary>
	/// マウスの押下をチェック
	/// </summary>
	/// <param name="keyNumber">マウスボタン番号(0:左,1:右,2:中)</param>
	/// <returns>押されているか</returns>
	bool PushMouse(int32_t keyNumber) const;

	/// <summary>
	/// キーのトリガーをチェック(押した瞬間true)
	/// </summary>
	/// <param name="keyNumber">キー番号( DIK_0 等)</param>
	/// <returns>トリガーか</returns>
	bool TriggerKey(BYTE keyNumber) const;

	/// <summary>
	/// キーのトリガーをチェック
	/// </summary>
	/// <param name="keyNumber">マウスボタン番号(0:左,1:右,2:中)</param>
	/// <returns>トリガーか</returns>
	bool TriggerMouse(int32_t keyNumber) const;

	/// <summary>
	/// キーを離した瞬間
	/// </summary>
	/// <param name="keyNumber"></param>
	/// <returns></returns>
	bool ExitKey(BYTE keyNumber) const;

	/// <summary>
	/// キーを離した瞬間
	/// </summary>
	/// <param name="keyNumber">マウスボタン番号(0:左,1:右,2:中)</param>
	/// <returns></returns>
	bool ExitMouse(int32_t keyNumber) const;

	/// <summary>
	/// 全キー情報取得
	/// </summary>
	/// <param name="keyStateBuf">全キー情報</param>
	const std::array<BYTE, 256>& GetAllKey() { return key_; }

	/// <summary>
	/// ホイールスクロール量を取得する
	/// </summary>
	/// <param name=""></param>
	/// <returns></returns>
	int32_t GetWheel() const;

	/// <summary>
	/// ホイールスクロール量を取得する
	/// </summary>
	/// <param name=""></param>
	/// <returns></returns>
	Vector2 GetMouseMove() const;
	/// <summary>
	/// 現在のジョイスティック状態を取得する
	/// </summary>
	/// <param name="stickNo">ジョイスティック番号</param>
	/// <param name="out">現在のジョイスティック状態</param>
	/// <returns>正しく取得できたか</returns>
	bool GetJoystickState(int32_t stickNo, DIJOYSTATE2& out) const;

	/// <summary>
	/// 前回のジョイスティック状態を取得する
	/// </summary>
	/// <param name="stickNo">ジョイスティック番号</param>
	/// <param name="out">前回のジョイスティック状態</param>
	/// <returns>正しく取得できたか</returns>
	bool GetJoystickStatePrevious(int32_t stickNo, DIJOYSTATE2& out) const;

	/// <summary>
	/// 現在のジョイスティック状態を取得する
	/// </summary>
	/// <param name="stickNo">ジョイスティック番号</param>
	/// <param name="out">現在のジョイスティック状態</param>
	/// <returns>正しく取得できたか</returns>
	bool GetJoystickState(int32_t stickNo, XINPUT_STATE& out) const;

	/// <summary>
	/// 前回のジョイスティック状態を取得する
	/// </summary>
	/// <param name="stickNo">ジョイスティック番号</param>
	/// <param name="out">前回のジョイスティック状態</param>
	/// <returns>正しく取得できたか</returns>
	bool GetJoystickStatePrevious(int32_t stickNo, XINPUT_STATE& out) const;

	bool IsControllerConnected() const;
private: // メンバ変数
	Input() = default;
	~Input() = default;
	Input(const Input&) = delete;
	const Input& operator=(const Input&) = delete;

	BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* instance, VOID* context);
	BOOL CALLBACK EnumJoystickObjectsCallback(const DIDEVICEOBJECTINSTANCE* instance, VOID* context);
private: // メンバ変数
	Microsoft::WRL::ComPtr<IDirectInput8> dInput_;
	Microsoft::WRL::ComPtr<IDirectInputDevice8> devKeyboard_;
	Microsoft::WRL::ComPtr<IDirectInputDevice8> devMouse_;
	std::vector<Joystick> devJoysticks_;

	DIMOUSESTATE2 mouse_;
	DIMOUSESTATE2 mousePre_;
	std::array<BYTE, 256> key_;
	std::array<BYTE, 256> keyPre_;
};
