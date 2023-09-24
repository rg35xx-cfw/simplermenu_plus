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
