#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <set>


enum class SettingId {
    // FIXME at some point we should decide how to deal with submenus
    //       and dynamic items (roms, etc.) and their ids
    None,

    // Menu main settings
    MAIN_FONT,
    MAIN_FONT_SIZE,
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
    ITEMS_PER_PAGE,
    LIST_OFFSET_X,
    LIST_OFFSET_Y,
    TITLE_FONT,
    TITLE_FONT_SIZE,
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
    QUIT

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
        {SettingId::MAIN_FONT, "Menu.mainFont"},
        {SettingId::MAIN_FONT_SIZE, "Menu.mainFontSize"},
        {SettingId::SCREEN_WIDTH, "Menu.screenWidth"},
        {SettingId::SCREEN_HEIGHT, "Menu.screenHeight"},
        {SettingId::SCREEN_DEPTH, "Menu.screenDepth"},
        {SettingId::HOME_PATH, "Menu.homePath"},
        {SettingId::ROMS_PATH, "Menu.romsPath"},
        {SettingId::THEME_PATH, "Menu.themePath"},
        {SettingId::ALIAS_PATH, "Menu.aliasPath"},
        {SettingId::THEME_NAME, "Menu.themeName"},
        {SettingId::SYSTEM_MENU_JSON, "Menu.systemMenuJSON"},
        {SettingId::EXCLUDED_EXTENSIONS, "Menu.excludedExtensions"},
        {SettingId::ITEMS_PER_PAGE, "Menu.itemsPerPage"},
        {SettingId::LIST_OFFSET_X, "Menu.listOffset_x"},
        {SettingId::LIST_OFFSET_Y, "Menu.listOffset_y"},
        {SettingId::TITLE_FONT, "Menu.titleFont"},
        {SettingId::TITLE_FONT_SIZE, "Menu.titleFontSize"},
        {SettingId::THUMBNAIL_WIDTH, "Menu.thumbnailWidth"},
        {SettingId::THUMBNAIL_HEIGHT, "Menu.thumbnailHeight"},

        // System settings
        {SettingId::VOLUME, "System.volume"}, 
        {SettingId::BRIGHTNESS, "System.brightness"}, 
        {SettingId::SCREEN_REFRESH, "System.screenRefresh"},
        {SettingId::SHOW_FPS, "System.showFPS"},
        {SettingId::OVERCLOCK, "System.overclock"},
        {SettingId::THEME, "System.theme"},
        {SettingId::USB_MODE, "System.usbMode"},
        {SettingId::WIFI, "System.wifi"},
        {SettingId::SETTINGS_PER_PAGE, "System.itemsPerPage"},
        {SettingId::QUIT, "System.quit"}
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

    std::string getIniKey(const SettingId& settingId) const;
    SettingId getSettingId(const std::string& key) const;
};
