#include "Controller.h"

using namespace std;

void ControlState::Initialize()
{
	ZeroMemory(keyboardCurrent, sizeof(char) * 256);
	ZeroMemory(keyboardLast, sizeof(char) * 256);
	ZeroMemory(keyboardTrigger, sizeof(bool) * 256);
	ZeroMemory(integratedSliderCurrent, sizeof(bool) * 16);
	ZeroMemory(integratedSliderLast, sizeof(bool) * 16);
	ZeroMemory(integratedSliderTrigger, sizeof(bool) * 16);
	ZeroMemory(integratedAir, sizeof(bool) * 4);

	sliderKeyboardInputCombinations[0] = { KEY_INPUT_A };
	sliderKeyboardInputCombinations[1] = { KEY_INPUT_Z };
	sliderKeyboardInputCombinations[2] = { KEY_INPUT_S };
	sliderKeyboardInputCombinations[3] = { KEY_INPUT_X };
	sliderKeyboardInputCombinations[4] = { KEY_INPUT_D };
	sliderKeyboardInputCombinations[5] = { KEY_INPUT_C };
	sliderKeyboardInputCombinations[6] = { KEY_INPUT_F };
	sliderKeyboardInputCombinations[7] = { KEY_INPUT_V };
	sliderKeyboardInputCombinations[8] = { KEY_INPUT_G };
	sliderKeyboardInputCombinations[9] = { KEY_INPUT_B };
	sliderKeyboardInputCombinations[10] = { KEY_INPUT_H };
	sliderKeyboardInputCombinations[11] = { KEY_INPUT_N };
	sliderKeyboardInputCombinations[12] = { KEY_INPUT_J };
	sliderKeyboardInputCombinations[13] = { KEY_INPUT_M };
	sliderKeyboardInputCombinations[14] = { KEY_INPUT_K };
	sliderKeyboardInputCombinations[15] = { KEY_INPUT_COMMA };
	airStringKeyboardInputCombinations[size_t(AirControlSource::AirUp)] = { KEY_INPUT_PGUP };
	airStringKeyboardInputCombinations[size_t(AirControlSource::AirDown)] = { KEY_INPUT_PGDN };
	airStringKeyboardInputCombinations[size_t(AirControlSource::AirHold)] = { KEY_INPUT_HOME };
	airStringKeyboardInputCombinations[size_t(AirControlSource::AirAction)] = { KEY_INPUT_END };
}

void ControlState::Terminate()
{
}

void ControlState::Update()
{
	// 生のキーボード入力
	memcpy_s(keyboardLast, sizeof(char) * 256, keyboardCurrent, sizeof(char) * 256);
	GetHitKeyStateAll(keyboardCurrent);
	for (auto i = 0; i < 256; i++) keyboardTrigger[i] = !keyboardLast[i] && keyboardCurrent[i];

	// キーボード入力スライダー
	for (auto i = 0; i < 16; i++) sliderKeyboardPrevious[i] = sliderKeyboardCurrent[i];
	auto snum = 0;
	for (const auto& targets : sliderKeyboardInputCombinations) {
		// 現状スライダーの一パネルあたり32キーしか割り当てられない
		uint32_t state = 0;
		uint32_t mask = 1;
		for (const auto& knum : targets) {
			if (keyboardCurrent[knum]) state |= mask;
			mask <<= 1;
			if (!mask) break;
		}
		sliderKeyboardCurrent[snum] = state;

		// トリガー判定は1個でも入力キーが増えればよしとする
		// <=> last で 0 のビットが current で 1 になっているものがある場合 true
		const auto diff = sliderKeyboardCurrent[snum] ^ sliderKeyboardPrevious[snum];
		sliderKeyboardTrigger[snum] = !!(diff & ~sliderKeyboardPrevious[snum]);
		++snum;
	}

	// キーボード入力エアストリング
	airStringKeyboard[size_t(AirControlSource::AirUp)] = false;
	airStringKeyboard[size_t(AirControlSource::AirDown)] = false;
	airStringKeyboard[size_t(AirControlSource::AirHold)] = false;
	airStringKeyboard[size_t(AirControlSource::AirAction)] = false;
	for (const auto& upkey : airStringKeyboardInputCombinations[size_t(AirControlSource::AirUp)]) {
		airStringKeyboard[size_t(AirControlSource::AirUp)] |= keyboardTrigger[upkey] ? 1 : 0;
	}
	for (const auto& downkey : airStringKeyboardInputCombinations[size_t(AirControlSource::AirDown)]) {
		airStringKeyboard[size_t(AirControlSource::AirDown)] |= keyboardTrigger[downkey];
	}
	for (const auto& upkey : airStringKeyboardInputCombinations[size_t(AirControlSource::AirHold)]) {
		airStringKeyboard[size_t(AirControlSource::AirHold)] |= !!keyboardCurrent[upkey];
	}
	for (const auto& actkey : airStringKeyboardInputCombinations[size_t(AirControlSource::AirAction)]) {
		airStringKeyboard[size_t(AirControlSource::AirAction)] |= keyboardTrigger[actkey];
	}

	// 統合化
	for (auto i = 0; i < 16; i++) integratedSliderLast[i] = integratedSliderCurrent[i];
	for (auto i = 0; i < 16; i++) integratedSliderCurrent[i] = !!sliderKeyboardCurrent[i];
	for (auto i = 0; i < 16; i++) integratedSliderTrigger[i] = sliderKeyboardTrigger[i];
	integratedAir[size_t(AirControlSource::AirUp)] = airStringKeyboard[size_t(AirControlSource::AirUp)];
	integratedAir[size_t(AirControlSource::AirDown)] = airStringKeyboard[size_t(AirControlSource::AirDown)];
	integratedAir[size_t(AirControlSource::AirHold)] = airStringKeyboard[size_t(AirControlSource::AirHold)];
	integratedAir[size_t(AirControlSource::AirAction)] = airStringKeyboard[size_t(AirControlSource::AirAction)];

	/*{
		lock_guard<mutex> lock(fingerMutex);
		for (auto &finger : currentFingers) integratedSliderCurrent[finger.second->SliderPosition] = 1;
	}*/
}

bool ControlState::GetTriggerState(const ControllerSource source, const int number)
{
	switch (source) {
	case ControllerSource::RawKeyboard:
		if (number < 0 || number >= 256) return false;
		return keyboardTrigger[number];
	case ControllerSource::IntegratedSliders:
		if (number < 0 || number >= 16) return false;
		return integratedSliderTrigger[number];
	case ControllerSource::RawTouch:
		return false;
	case ControllerSource::IntegratedAir:
		if (number < 0 || number >= 4) return false;
		return integratedAir[number];
	}
	return false;
}

bool ControlState::GetCurrentState(const ControllerSource source, const int number)
{
	switch (source) {
	case ControllerSource::RawKeyboard:
		if (number < 0 || number >= 256) return false;
		return !!keyboardCurrent[number];
	case ControllerSource::IntegratedSliders:
		if (number < 0 || number >= 16) return false;
		return integratedSliderCurrent[number];
	case ControllerSource::RawTouch:
		return false;
	case ControllerSource::IntegratedAir:
		if (number < 0 || number >= 4) return false;
		return integratedAir[number];
	}
	return false;
}

bool ControlState::GetLastState(const ControllerSource source, const int number)
{
	switch (source) {
	case ControllerSource::RawKeyboard:
		if (number < 0 || number >= 256) return false;
		return !!keyboardLast[number];
	case ControllerSource::IntegratedSliders:
		if (number < 0 || number >= 16) return false;
		return integratedSliderLast[number];
	case ControllerSource::RawTouch:
		return false;
	case ControllerSource::IntegratedAir:
		if (number < 0 || number >= 4) return false;
		return integratedAir[number];
	}
	return false;
}

void ControlState::SetSliderKeyCombination(const int sliderNumber, const vector<int> & keys)
{
	if (sliderNumber < 0 || sliderNumber >= 16) return;
	if (keys.size() > 8) return;
	sliderKeyboardInputCombinations[sliderNumber] = keys;
}

void ControlState::SetAirStringKeyCombination(const int airNumber, const vector<int> & keys)
{
	if (airNumber < 0 || airNumber >= 4) return;
	if (keys.size() > 8) return;
	airStringKeyboardInputCombinations[airNumber] = keys;
}
