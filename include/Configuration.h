#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <map>
#include <set>
#include <vector>
#include <memory>

struct ConsoleData {
    std::string name;
    std::vector<std::string> execs;
    std::vector<std::string> romExts;
    std::vector<std::string> romDirs;
};


enum class SettingId {
    // FIXME at some point we should decide how to deal with submenus
    //       and dynamic items (roms, etc.) and their ids
    None,

    // Menu main settings
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    SCREEN_DEPTH,
    HOME_PATH,
    ROMS_PATH,
    THEME_PATH,
    ALIAS_PATH,
    THEME_NAME,
    SYSTEM_MENU_JSON,
    EXCLUDED_EXTENSIONS,
    THUMBNAIL_WIDTH,
    THUMBNAIL_HEIGHT,

    // System Settings
    VOLUME,
    BRIGHTNESS,
    SCREEN_REFRESH,
    SHOW_FPS,
    OVERCLOCK,
    THEME,
    USB_MODE,
    WIFI,
    SETTINGS_PER_PAGE,
    QUIT,

    // Control Settings
    BTN_A,
    BTN_B,
    BTN_X,
    BTN_Y,
    BTN_L1,
    BTN_L2,
    BTN_R1,
    BTN_R2,
    BTN_UP,
    BTN_DOWN,
    BTN_RIGHT,
    BTN_LEFT,
    BTN_START,
    BTN_SELECT

};

inline std::ostream& operator<<(std::ostream& o, SettingId c) {
    std::cout << static_cast<int>(c);
    return o;
}

inline std::string operator+(std::string s, SettingId c) {
    return s + std::to_string(static_cast<int>(c));
}


class Configuration {
private:
    std::unordered_map<SettingId, std::string> configValues;

    // TODO should this be constant?
    std::unordered_map<SettingId, std::string> idToStr {
        // Menu main settings
        {SettingId::SCREEN_WIDTH, "MENU.screenWidth"},
        {SettingId::SCREEN_HEIGHT, "MENU.screenHeight"},
        {SettingId::SCREEN_DEPTH, "MENU.screenDepth"},
        {SettingId::HOME_PATH, "MENU.homePath"},
        {SettingId::ROMS_PATH, "MENU.romsPath"},
        {SettingId::THEME_PATH, "MENU.themePath"},
        {SettingId::ALIAS_PATH, "MENU.aliasPath"},
        {SettingId::THEME_NAME, "MENU.themeName"},
        {SettingId::SYSTEM_MENU_JSON, "MENU.systemMenuJSON"},
        {SettingId::EXCLUDED_EXTENSIONS, "MENU.excludedExtensions"},
        {SettingId::THUMBNAIL_WIDTH, "MENU.thumbnailWidth"},
        {SettingId::THUMBNAIL_HEIGHT, "MENU.thumbnailHeight"},

        // System settings
        {SettingId::VOLUME, "SYSTEM.volume"}, 
        {SettingId::BRIGHTNESS, "SYSTEM.brightness"}, 
        {SettingId::SCREEN_REFRESH, "SYSTEM.screenRefresh"},
        {SettingId::SHOW_FPS, "SYSTEM.showFPS"},
        {SettingId::OVERCLOCK, "SYSTEM.overclock"},
        {SettingId::THEME, "SYSTEM.theme"},
        {SettingId::USB_MODE, "SYSTEM.usbMode"},
        {SettingId::WIFI, "SYSTEM.wifi"},
        {SettingId::SETTINGS_PER_PAGE, "SYSTEM.itemsPerPage"},
        {SettingId::QUIT, "SYSTEM.quit"},

        // Control Settings
        {SettingId::BTN_A, "CONTROLS.BTN_A"}, 
        {SettingId::BTN_B, "CONTROLS.BTN_B"}, 
        {SettingId::BTN_X, "CONTROLS.BTN_X"}, 
        {SettingId::BTN_Y, "CONTROLS.BTN_Y"}, 
        {SettingId::BTN_L1, "CONTROLS.BTN_L1"}, 
        {SettingId::BTN_L2, "CONTROLS.BTN_L2"}, 
        {SettingId::BTN_R1, "CONTROLS.BTN_R1"}, 
        {SettingId::BTN_R2, "CONTROLS.BTN_R2"}, 
        {SettingId::BTN_UP, "CONTROLS.BTN_UP"}, 
        {SettingId::BTN_DOWN, "CONTROLS.BTN_DOWN"}, 
        {SettingId::BTN_LEFT, "CONTROLS.BTN_LEFT"}, 
        {SettingId::BTN_RIGHT, "CONTROLS.BTN_RIGHT"}, 
        {SettingId::BTN_START, "CONTROLS.BTN_START"}, 
        {SettingId::BTN_SELECT, "CONTROLS.BTN_SELECT"}, 



    };

    std::unordered_map<std::string, SettingId> strToId;

    Configuration(); // Private constructor for singleton

public:
    // Delete copy constructor and assignment operator
    Configuration(const Configuration&) = delete;
    Configuration& operator=(const Configuration&) = delete;

    // Static method to get the instance
    static Configuration& getInstance();

    void setValue(const SettingId& id, const std::string& value);

    std::string getValue(const SettingId& settingId) const;
    bool getBoolValue(const SettingId& settingId) const;
    int getIntValue(const SettingId& settingId) const;

    std::set<std::string> getStringList(const SettingId& settingId, 
                                        char delimiter = ',') const;
    std::string getThemePath() const;
    std::map<std::string, ConsoleData> parseIniFile(const std::string& iniPath);

    std::string getIniKey(const SettingId& settingId) const;
    SettingId getSettingId(const std::string& key) const;
};
