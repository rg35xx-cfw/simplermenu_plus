#pragma once
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include <map>
#include <set>
#include <vector>
#include <memory>

#include "State.h"

struct ConsoleData {
    std::string name;
    std::vector<std::string> execs;
    std::vector<std::string> romExts;
    std::vector<std::string> romDirs;
};

struct SettingsMenuItem {
    std::string id;
    std::string type;
    std::string title;
    std::vector<std::string> options; // Empty for menu items that don't have options

    // Constructor
    SettingsMenuItem(const std::string& id, const std::string& type, 
             const std::string& title, const std::vector<std::string>& options = {})
        : id(id), type(type), title(title), options(options) {}
};

// class Configuration : public ISettingsSubject {
class Configuration {
private:

    std::string configIniFilepath;
    std::string stateFilepath;

    boost::property_tree::ptree mainPt;

public:

    /////////
    // CONFIG.INI
    ////////

    static const std::string GLOBAL;
    static const std::string APPLICATION;
    static const std::string SYSTEM;
    static const std::string GAME;

    // CONFIG . GLOBAL section
    static const std::string ALIAS_PATH;
    static const std::string HOME_PATH;
    static const std::string THEME_PATH;
    static const std::string IMAGES_PATH;
    static const std::string SCREEN_WIDTH;
    static const std::string SCREEN_HEIGHT;
    static const std::string SCREEN_DEPTH;
    static const std::string GLOBAL_CACHE;

    // CONFIG . SYSTEM section
    static const std::string VOLUME;
    static const std::string BRIGHTNESS;
    static const std::string SCREEN_REFRESH;
    static const std::string SHOW_FPS;
    static const std::string OVERCLOCK_VALUES;
    static const std::string OVERCLOCK;
    static const std::string THEME;
    static const std::string USB_MODE_VALUES;
    static const std::string USB_MODE;
    static const std::string WIFI;
    static const std::string ROTATION;
    static const std::string LANGUAGE;
    static const std::string UPDATE_CACHES;
    static const std::string RESTART;
    static const std::string QUIT;

    // CONFIG . FOLDER section
    static const std::string CORE_SELECTION;

    // CONFIG . GAME section
    static const std::string ROM_OVERCLOCK;
    static const std::string ROM_AUTOSTART;
    static const std::string CORE_OVERRIDE;

    /////////
    // THEME.INI
    /////////

    // THEME . DEFAULT section
    static const std::string SEL_ITEM_FONT_COLOR;
    static const std::string ITEMS_FONT_COLOR;
    static const std::string THEME_BACKGROUND;

    // THEME . GENERAL section
    static const std::string ART_X;
    static const std::string ART_Y;
    static const std::string ART_MAX_W;
    static const std::string ART_MAX_H;
    static const std::string ART_TXT_DIST_FROM_PIC;
    static const std::string ART_TXT_LINE_SEP;
    static const std::string DISPLAY_GAME_COUNT;
    static const std::string GAME_COUNT_FONT_COLOR;
    static const std::string GAME_COUNT_X;
    static const std::string GAME_COUNT_Y;
    static const std::string GAME_COUNT_ALIGNMENT;
    static const std::string GAME_LIST_X;
    static const std::string GAME_LIST_Y;
    static const std::string ITEMS;
    static const std::string ITEMS_SEPARATION;
    static const std::string TEXT1_X;
    static const std::string TEXT1_Y;
    static const std::string TEXT1_ALIGNMENT;
    static const std::string TEXT2_X;
    static const std::string TEXT2_Y;
    static const std::string TEXT2_ALIGNMENT;
    static const std::string THEME_FONT;

    /////////
    // <SECTION>.INI
    /////////

    static const std::string CONSOLES;

    // <SECTION> . CONSOLES section
    static const std::string CONSOLES_LIST;

    // <SECTION> . <CONSOLE_NAME> section
    static const std::string CONSOLE_EXECS;
    static const std::string CONSOLE_ROM_DIRS;
    static const std::string CONSOLE_ROM_EXTS;
    static const std::string CONSOLE_ALIAS_FILE;

    //////////
    // Savestate.json
    //////////
    static const std::string CURRENT_MENU_LEVEL;
    static const std::string CURRENT_SECTION_INDEX;
    static const std::string CURRENT_FOLDER_INDEX;
    static const std::string CURRENT_ROM_INDEX;
    static const std::string LAUNCHER_CALLBACK;


    Configuration(const std::string& configIniFilepath,
                  const std::string& stateFilepath);

    void set(const std::string& id, const std::string& value);

    std::string get(const std::string& id) const;
    bool getBool(const std::string& id) const;
    int getInt(const std::string& id) const;

    std::set<std::string> getList(const std::string& id, 
                                  const char delimiter = ',') const;
    std::string getThemePath() const;
    std::map<std::string, ConsoleData> parseIniFile(const std::string& iniPath);

    void saveConfigIni();

    State loadState();
    void saveState(const State& state);

    int getSectionSize(std::string section) {
        return mainPt.get_child(section).size();
    }

    boost::property_tree::ptree getSection(const std::string& section) {
        return mainPt.get_child(section);
    }

    std::string getKeyByIndex(const std::string& sectionName, std::size_t index) {
        auto section = mainPt.get_child(sectionName);
        std::size_t currentIndex = 0;
        for (const auto& kv : section) {
            if (currentIndex == index) {
                return kv.first;
            }
            ++currentIndex;
        }
        throw std::out_of_range("Index out of range");
    }

    std::string getValueByIndex(const std::string& sectionName, std::size_t index) {
        auto section = mainPt.get_child(sectionName);
        std::size_t currentIndex = 0;
        for (const auto& kv : section) {
            if (currentIndex == index) {
                return kv.second.get_value<std::string>();
            }
            ++currentIndex;
        }
        throw std::out_of_range("Index out of range");
    }

};
