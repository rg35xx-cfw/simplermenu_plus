#pragma once

#include "Configuration.h"
#include <SDL/SDL.h>
#include <unordered_map>
#include <string>

enum ControlMap {
    CMD_UP,
    CMD_DOWN,
    CMD_LEFT,
    CMD_RIGHT,
    CMD_ENTER,
    CMD_BACK,
    CMD_SYS_SETTINGS,
    CMD_ROM_SETTINGS,
    CMD_NONE
};

class ControlMapping {
private:
    std::unordered_map<std::string, int> controls;
    Configuration cfg;

public:
    // Constructor to initialize the mapping from config
    ControlMapping(Configuration& cfg);

    // Get the SDL key value for a given control name
    int getControl(const std::string& controlName) const;

    // Convert keyboard and joystick commands to the controlmap enum
    ControlMap convertCommand(const SDL_Event& event);
};

