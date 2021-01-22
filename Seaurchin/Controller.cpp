#include "Controller.h"

using namespace std;

#ifndef _WIN32
#define ZeroMemory(ptr, len) memset((ptr), 0, (len)) 
#endif

void ControlState::Initialize()
{
    ZeroMemory(keyboardCurrent, sizeof(bool) * 256);
    ZeroMemory(keyboardLast, sizeof(bool) * 256);
    ZeroMemory(keyboardTrigger, sizeof(bool) * 256);

    sliderKeybindings[0] = {};
    sliderKeybindings[1] = {};
    sliderKeybindings[2] = {};
    sliderKeybindings[3] = {};
    sliderKeybindings[4] = {};
    sliderKeybindings[5] = {};
    sliderKeybindings[6] = {};
    sliderKeybindings[7] = {};
    sliderKeybindings[8] = {};
    sliderKeybindings[9] = {};
    sliderKeybindings[10] = {};
    sliderKeybindings[11] = {};
    sliderKeybindings[12] = {};
    sliderKeybindings[13] = {};
    sliderKeybindings[14] = {};
    sliderKeybindings[15] = {};
}

void ControlState::Terminate()
{
}

void ControlState::Update()
{
    // 生のキーボード入力
    memcpy(keyboardLast, keyboardCurrent, sizeof(bool) * 256);
    memset(keyboardTrigger, 0, 256 * sizeof(bool));

    for(int i = 0; i < airKeybindings.size(); i++){
        airLast[i] = airCurrent[i];
        airTrigger[i] = 0;
    }

    while(libevdev_has_event_pending(inputDevice)){
        struct input_event * evt = NULL;
        // TODO handle desyncs
        if(libevdev_next_event(inputDevice, LIBEVDEV_READ_FLAG_NORMAL, evt) == LIBEVDEV_READ_STATUS_SUCCESS){
            if(evt->value){
                // key was pressed
                keyboardTrigger[evt->code] = true;
            }
            keyboardCurrent[evt->code] = evt->value;
        }
    }

    auto sliderLane = 0;

    for (const auto& keysForLane : sliderKeybindings) {
        for (const auto &key : keysForLane) {
            // Held if at least one of the keys is held
            if(keyboardCurrent[key]) {
                sliderCurrent[sliderLane] = true;
            }

            // Triggered if at least one of the keys is triggered
            if(keyboardTrigger[key]){
                sliderTrigger[sliderLane] = true;
            }
        }
        ++sliderLane;
    }

    auto airLane = 0;
    for(const auto &key : airKeybindings){
        if(keyboardCurrent[key]){
            airCurrent[airLane] = true;
        }

        if(keyboardTrigger[key]){
            airTrigger[airLane] = true;
        }
    }
}

bool ControlState::GetTriggerState(const ControllerSource source, const int number)
{
    switch (source) {
        case ControllerSource::RawKeyboard:
            if (number < 0 || number >= 256) return false;
            return keyboardTrigger[number];
        case ControllerSource::Slider:
            if (number < 0 || number >= 16) return false;
            return sliderTrigger[number];
        case ControllerSource::Air:
            // The entire air sensor is triggered if there is any change in its state
            if(number == -1){
                for(int i = 0; i < airKeybindings.size(); i++){
                    if(airLast[i] != airCurrent[i]){
                        return true;
                    }
                }
                return false;
            }
            else{
                if (number < 0 || number >= airKeybindings.size()) return false;
                return airKeybindings[number];
            }
    }
    return false;
}

bool ControlState::GetCurrentState(const ControllerSource source, const int number)
{
    switch (source) {
        case ControllerSource::RawKeyboard:
            if (number < 0 || number >= 256) return false;
            return keyboardCurrent[number];
        case ControllerSource::Slider:
            if (number < 0 || number >= 16) return false;
            return sliderCurrent[number];
        case ControllerSource::Air:
            // The entire air sensor is held if any key is held
            if (number == -1){
                for(bool val : airCurrent){
                    if(val) return true;
                }
                return false;
            }
            else{
                if (number < 0 || number >= airKeybindings.size()) return false;
                return airCurrent[number];
            }
    }
    return false;
}

bool ControlState::GetLastState(const ControllerSource source, const int number)
{
    switch (source) {
        case ControllerSource::RawKeyboard:
            if (number < 0 || number >= 256) return false;
            return keyboardLast[number];
        case ControllerSource::Slider:
            if (number < 0 || number >= 16) return false;
            return sliderLast[number];
        case ControllerSource::Air:
            if (number == -1){
                for(bool val : airLast){
                    if(val) return true;
                }
                return false;
            }
            else{
                if (number < 0 || number >= airKeybindings.size()) return false;
                return airLast[number];
            }
    }
    return false;
}

void ControlState::SetSliderKeybindings(const int sliderNumber, const vector<int>& keys)
{
    if (sliderNumber < 0 || sliderNumber >= 16) return;
    if (keys.size() > 8) return;
    sliderKeybindings[sliderNumber] = keys;
}

void ControlState::SetAirKeybindings(const vector<int>& keys)
{
    airKeybindings = keys;
    airCurrent = std::vector<bool>(keys.size());
    airLast = std::vector<bool>(keys.size());
    airTrigger = std::vector<bool>(keys.size());
}

bool ControlState::SetInputDevice(int fp)
{
    if(inputDevice != NULL){
        int old_fd = libevdev_get_fd(inputDevice);
        libevdev_free(inputDevice);
        close(old_fd);
    }

    if(libevdev_new_from_fd(fp, &inputDevice) > 0){
        return true;
    }

    return false;
}