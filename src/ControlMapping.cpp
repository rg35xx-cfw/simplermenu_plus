#include "ControlMapping.h"

ControlMapping::ControlMapping() {
    Configuration& cfg = Configuration::getInstance();

    controls["KEY_A"] = cfg.getIntValue(SettingId::KEY_A);
    controls["KEY_B"] = cfg.getIntValue(SettingId::KEY_B);
    controls["KEY_X"] = cfg.getIntValue(SettingId::KEY_X);
    controls["KEY_Y"] = cfg.getIntValue(SettingId::KEY_Y);
    controls["KEY_L1"] = cfg.getIntValue(SettingId::KEY_L1);
    controls["KEY_L2"] = cfg.getIntValue(SettingId::KEY_L2);
    controls["KEY_R1"] = cfg.getIntValue(SettingId::KEY_R1);
    controls["KEY_R2"] = cfg.getIntValue(SettingId::KEY_R2);
    controls["KEY_UP"] = cfg.getIntValue(SettingId::KEY_UP);
    controls["KEY_DOWN"] = cfg.getIntValue(SettingId::KEY_DOWN);
    controls["KEY_LEFT"] = cfg.getIntValue(SettingId::KEY_LEFT);
    controls["KEY_RIGHT"] = cfg.getIntValue(SettingId::KEY_RIGHT);
    controls["KEY_START"] = cfg.getIntValue(SettingId::KEY_START);
    controls["KEY_SELECT"] = cfg.getIntValue(SettingId::KEY_SELECT);

    controls["BTN_A"] = cfg.getIntValue(SettingId::BTN_A);
    controls["BTN_B"] = cfg.getIntValue(SettingId::BTN_B);
    controls["BTN_X"] = cfg.getIntValue(SettingId::BTN_X);
    controls["BTN_Y"] = cfg.getIntValue(SettingId::BTN_Y);
    controls["BTN_L1"] = cfg.getIntValue(SettingId::BTN_L1);
    controls["BTN_L2"] = cfg.getIntValue(SettingId::BTN_L2);
    controls["BTN_R1"] = cfg.getIntValue(SettingId::BTN_R1);
    controls["BTN_R2"] = cfg.getIntValue(SettingId::BTN_R2);
    controls["BTN_UP"] = cfg.getIntValue(SettingId::BTN_UP);
    controls["BTN_DOWN"] = cfg.getIntValue(SettingId::BTN_DOWN);
    controls["BTN_LEFT"] = cfg.getIntValue(SettingId::BTN_LEFT);
    controls["BTN_RIGHT"] = cfg.getIntValue(SettingId::BTN_RIGHT);
    controls["BTN_START"] = cfg.getIntValue(SettingId::BTN_START);
    controls["BTN_SELECT"] = cfg.getIntValue(SettingId::BTN_SELECT);
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
