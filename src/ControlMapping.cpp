#include "ControlMapping.h"

ControlMapping::ControlMapping() {
    Configuration& cfg = Configuration::getInstance();

    controls["A"] = cfg.getIntValue(SettingId::BTN_A);
    controls["B"] = cfg.getIntValue(SettingId::BTN_B);
    controls["X"] = cfg.getIntValue(SettingId::BTN_X);
    controls["Y"] = cfg.getIntValue(SettingId::BTN_Y);
    controls["L1"] = cfg.getIntValue(SettingId::BTN_L1);
    controls["L2"] = cfg.getIntValue(SettingId::BTN_L2);
    controls["R1"] = cfg.getIntValue(SettingId::BTN_R1);
    controls["R2"] = cfg.getIntValue(SettingId::BTN_R2);
    controls["UP"] = cfg.getIntValue(SettingId::BTN_UP);
    controls["DOWN"] = cfg.getIntValue(SettingId::BTN_DOWN);
    controls["LEFT"] = cfg.getIntValue(SettingId::BTN_LEFT);
    controls["RIGHT"] = cfg.getIntValue(SettingId::BTN_RIGHT);
    controls["START"] = cfg.getIntValue(SettingId::BTN_START);
    controls["SELECT"] = cfg.getIntValue(SettingId::BTN_SELECT);
}

ControlMapping& ControlMapping::getInstance() {
    static ControlMapping instance;  // Lazy initialized and destroyed on program exit
    return instance;
}

int ControlMapping::getControl(const std::string& controlName) const {
    auto it = controls.find(controlName);
    if (it != controls.end()) {
        return it->second;
    } else {
        return -1;  // Return -1 if the control name is not found
    }
}
