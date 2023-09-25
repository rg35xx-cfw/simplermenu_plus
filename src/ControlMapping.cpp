#include "ControlMapping.h"

ControlMapping::ControlMapping(Configuration& cfg)
    : cfg(cfg) {

    controls["KEY_A"] = cfg.getInt("CONTROLS.KEY_A");
    controls["KEY_B"] = cfg.getInt("CONTROLS.KEY_B");
    controls["KEY_X"] = cfg.getInt("CONTROLS.KEY_X");
    controls["KEY_Y"] = cfg.getInt("CONTROLS.KEY_Y");
    controls["KEY_L1"] = cfg.getInt("CONTROLS.KEY_L1");
    controls["KEY_L2"] = cfg.getInt("CONTROLS.KEY_L2");
    controls["KEY_R1"] = cfg.getInt("CONTROLS.KEY_R1");
    controls["KEY_R2"] = cfg.getInt("CONTROLS.KEY_R2");
    controls["KEY_UP"] = cfg.getInt("CONTROLS.KEY_UP");
    controls["KEY_DOWN"] = cfg.getInt("CONTROLS.KEY_DOWN");
    controls["KEY_LEFT"] = cfg.getInt("CONTROLS.KEY_LEFT");
    controls["KEY_RIGHT"] = cfg.getInt("CONTROLS.KEY_RIGHT");
    controls["KEY_START"] = cfg.getInt("CONTROLS.KEY_START");
    controls["KEY_SELECT"] = cfg.getInt("CONTROLS.KEY_SELECT");

    controls["BTN_A"] = cfg.getInt("CONTROLS.BTN_A");
    controls["BTN_B"] = cfg.getInt("CONTROLS.BTN_B");
    controls["BTN_X"] = cfg.getInt("CONTROLS.BTN_X");
    controls["BTN_Y"] = cfg.getInt("CONTROLS.BTN_Y");
    controls["BTN_L1"] = cfg.getInt("CONTROLS.BTN_L1");
    controls["BTN_L2"] = cfg.getInt("CONTROLS.BTN_L2");
    controls["BTN_R1"] = cfg.getInt("CONTROLS.BTN_R1");
    controls["BTN_R2"] = cfg.getInt("CONTROLS.BTN_R2");
    controls["BTN_UP"] = cfg.getInt("CONTROLS.BTN_UP");
    controls["BTN_DOWN"] = cfg.getInt("CONTROLS.BTN_DOWN");
    controls["BTN_LEFT"] = cfg.getInt("CONTROLS.BTN_LEFT");
    controls["BTN_RIGHT"] = cfg.getInt("CONTROLS.BTN_RIGHT");
    controls["BTN_START"] = cfg.getInt("CONTROLS.BTN_START");
    controls["BTN_SELECT"] = cfg.getInt("CONTROLS.BTN_SELECT");
}

int ControlMapping::getControl(const std::string& controlName) const {
    auto it = controls.find(controlName);
    if (it != controls.end()) {
        return it->second;
    } else {
        return -1;  // Return -1 if the control name is not found
    }
}

ControlMap ControlMapping::convertCommand(const SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == getControl("KEY_A")) return CMD_ENTER;
        if (event.key.keysym.sym == getControl("KEY_B")) return CMD_BACK;
        if (event.key.keysym.sym == getControl("KEY_UP")) return CMD_UP;
        if (event.key.keysym.sym == getControl("KEY_DOWN")) return CMD_DOWN;
        if (event.key.keysym.sym == getControl("KEY_LEFT")) return CMD_LEFT;
        if (event.key.keysym.sym == getControl("KEY_RIGHT")) return CMD_RIGHT;
        if (event.key.keysym.sym == getControl("KEY_START")) return CMD_SYS_SETTINGS;
        if (event.key.keysym.sym == getControl("KEY_SELECT")) return CMD_ROM_SETTINGS;
    }
    if (event.type == SDL_JOYBUTTONDOWN) {
        if (event.jbutton.button == getControl("BTN_A")) return CMD_ENTER;
        if (event.jbutton.button == getControl("BTN_B")) return CMD_BACK;
        if (event.jbutton.button == getControl("BTN_UP")) return CMD_UP;
        if (event.jbutton.button == getControl("BTN_DOWN")) return CMD_DOWN;
        if (event.jbutton.button == getControl("BTN_LEFT")) return CMD_LEFT;
        if (event.jbutton.button == getControl("BTN_RIGHT")) return CMD_RIGHT;
        if (event.jbutton.button == getControl("BTN_START")) return CMD_SYS_SETTINGS;
        if (event.jbutton.button == getControl("BTN_SELECT")) return CMD_ROM_SETTINGS;
    } else if (event.type == SDL_JOYAXISMOTION) {
        // axis mappings
        int axis = event.jaxis.axis;
        int value = event.jaxis.value;
        //std::cout << "Joystick AXIS " << axis << " - value: " << value << std::endl;
        if (axis == 0) {
            if (value < 258) {
                return CMD_DOWN;
            } else if (value > 258) {
                return CMD_UP;
            } else if (value == 258) {
                std::cout << "AXIS U/D ZERO\n" << std::endl;
            }
        } else if (axis == 1) {
            if (value < 258) {
                return CMD_RIGHT;
            } else if (value > 258) {
                return CMD_LEFT;
            } else if (value == 258) {
                std::cout << "AXIS L/R ZERO\n" << std::endl;
            }
        }
    } else if (event.type == SDL_JOYHATMOTION) {
        // joystick HAT0, etc.
        if (event.jhat.value == SDL_HAT_UP) return CMD_UP;
    }

    return CMD_NONE; 
}

