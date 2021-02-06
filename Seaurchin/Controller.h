#pragma once

#include <vector>
#include <stdint.h>
#include <unistd.h>
#include <cstring>
#include <SDL2/SDL.h>

#define AIRSENSOR_ALL -1

enum class ControllerSource {
    RawKeyboard,
    Slider,
    Air,
};

class ControlState final {
private:
    bool keyboardCurrent[256];
    bool keyboardLast[256];
    bool keyboardTrigger[256];

    bool sliderCurrent[16];
    bool sliderLast[16];
    bool sliderTrigger[16];

    std::vector<bool> airCurrent;
    std::vector<bool> airLast;
    std::vector<bool> airTrigger;

    std::vector<int> sliderKeybindings[16];
    std::vector<int> airKeybindings;
public:
    void Initialize();
    void Terminate();
    bool Update();

    bool GetTriggerState(ControllerSource source, int number);
    bool GetCurrentState(ControllerSource source, int number);
    bool GetLastState(ControllerSource source, int number);
    float GetAirPosition();

    void SetSliderKeybindings(int sliderNumber, const std::vector<int>& keys);
    void SetAirKeybindings(const std::vector<int>& keys);
};