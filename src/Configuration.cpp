#include "Configuration.h"
#include "Exception.h"
#include <iostream>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string.hpp>


/////////
// CONFIG.INI
////////

const std::string Configuration::GLOBAL = std::string("GLOBAL");
const std::string Configuration::SYSTEM = std::string("SYSTEM");
const std::string Configuration::FOLDER = std::string("FOLDER");
const std::string Configuration::GAME = std::string("GAME");

// CONFIG . GLOBAL section
const std::string Configuration::ALIAS_PATH = std::string("GLOBAL.aliasPath");
const std::string Configuration::HOME_PATH = std::string("GLOBAL.homePath");
const std::string Configuration::THEME_PATH = std::string("GLOBAL.themePath");
const std::string Configuration::SCREEN_WIDTH = std::string("GLOBAL.screenWidth");
const std::string Configuration::SCREEN_HEIGHT = std::string("GLOBAL.screenHeight");
const std::string Configuration::SCREEN_DEPTH = std::string("GLOBAL.screenDepth");


// CONFIG . SYSTEM section
const std::string Configuration::VOLUME = std::string("SYSTEM.volume");
const std::string Configuration::BRIGHTNESS = std::string("SYSTEM.brightness");
const std::string Configuration::SCREEN_REFRESH = std::string("SYSTEM.screenRefresh");
const std::string Configuration::SHOW_FPS = std::string("SYSTEM.showFPS");
const std::string Configuration::OVERCLOCK = std::string("SYSTEM.overclock");
const std::string Configuration::OVERCLOCK_VALUES = std::string("GLOBAL.overclockValues");
const std::string Configuration::THEME = std::string("SYSTEM.theme");
const std::string Configuration::USB_MODE = std::string("SYSTEM.usbMode");
const std::string Configuration::USB_MODE_VALUES = std::string("GLOBAL.usbModeValues");
const std::string Configuration::WIFI = std::string("SYSTEM.wifi");
const std::string Configuration::ROTATION = std::string("SYSTEM.rotation");
const std::string Configuration::LANGUAGE = std::string("SYSTEM.language");
const std::string Configuration::UPDATE_CACHES = std::string("SYSTEM.updateCaches");
const std::string Configuration::SAVE_SETTINGS = std::string("SYSTEM.saveSettings");
const std::string Configuration::RESTART = std::string("SYSTEM.restart");
const std::string Configuration::QUIT = std::string("SYSTEM.quit");

// CONFIG . FOLDER section
const std::string Configuration::CORE_SELECTION = std::string("FOLDER.coreSelection");

// CONFIG . GAME section
const std::string Configuration::ROM_OVERCLOCK = std::string("GAME.romOverclock");
const std::string Configuration::ROM_AUTOSTART = std::string("GAME.romAutostart");
const std::string Configuration::CORE_OVERRIDE = std::string("GAME.coreOverride");


/////////
// THEME.INI
/////////

// THEME . DEFAULT section
const std::string Configuration::SEL_ITEM_FONT_COLOR = std::string("DEFAULT.selected_item_font_color");
const std::string Configuration::ITEMS_FONT_COLOR = std::string("DEFAULT.items_font_color");
const std::string Configuration::THEME_BACKGROUND = std::string("DEFAULT.background");

// THEME . GENERAL section
const std::string Configuration::ART_X = std::string("GENERAL.art_x");
const std::string Configuration::ART_Y = std::string("GENERAL.art_y");
const std::string Configuration::ART_MAX_W = std::string("GENERAL.art_max_w");
const std::string Configuration::ART_MAX_H = std::string("GENERAL.art_max_h");
const std::string Configuration::ART_TXT_DIST_FROM_PIC = std::string("GENERAL.art_text_distance_from_picture");
const std::string Configuration::ART_TXT_LINE_SEP = std::string("GENERAL.art_text_line_separation");
const std::string Configuration::DISPLAY_GAME_COUNT = std::string("GENERAL.display_game_count");
const std::string Configuration::GAME_COUNT_ALIGNMENT = std::string("GENERAL.game_count_alignment");
const std::string Configuration::GAME_COUNT_FONT_COLOR = std::string("GENERAL.game_count_font_color");
const std::string Configuration::GAME_COUNT_X = std::string("GENERAL.game_count_x");
const std::string Configuration::GAME_COUNT_Y = std::string("GENERAL.game_count_y");
const std::string Configuration::GAME_LIST_X = std::string("GENERAL.game_list_x");
const std::string Configuration::GAME_LIST_Y = std::string("GENERAL.game_list_y");
const std::string Configuration::ITEMS = std::string("GENERAL.items");
const std::string Configuration::ITEMS_SEPARATION = std::string("GENERAL.items_separation");
const std::string Configuration::TEXT1_X= std::string("GENERAL.text1_x");
const std::string Configuration::TEXT1_Y= std::string("GENERAL.text1_y");
const std::string Configuration::TEXT1_ALIGNMENT= std::string("GENERAL.text1_alignment");
const std::string Configuration::TEXT2_X= std::string("GENERAL.text2_x");
const std::string Configuration::TEXT2_Y = std::string("GENERAL.text2_y");
const std::string Configuration::TEXT2_ALIGNMENT = std::string("GENERAL.text2_alignment");
const std::string Configuration::THEME_FONT = std::string("GENERAL.font");



Configuration::Configuration(const std::string& configIniFilepath) 
    : configIniFilepath(configIniFilepath) {

    // Load values from .ini file using Boost.PropertyTree
    boost::property_tree::ini_parser::read_ini(configIniFilepath, mainPt);
}

void Configuration::set(const std::string& id, const std::string& value) {
    mainPt.put(id, value);
}

std::string Configuration::get(const std::string& id) const {
    // TODO exceptions  ptree_bad_path, ptree_bad_data, both from ptree_error 
    return mainPt.get<std::string>(id);
}

bool Configuration::getBool(const std::string& id) const {
    // TODO exceptions  ptree_bad_path, ptree_bad_data, both from ptree_error
    return mainPt.get<bool>(id);

    // std::string value = boost::algorithm::to_lower_copy(get(id));
    // try {
    //     return value == "on";
    // } catch (const std::exception& e) {
    //     throw ValueConversionException("Error converting value for id '"
    //         + id + "' to boolean: " + e.what());
    // }
}

int Configuration::getInt(const std::string& id) const {
    // TODO exceptions  ptree_bad_path, ptree_bad_data, both from ptree_error
    return mainPt.get<int>(id);
    // std::string value = getValue(id);
    // try {
    //     return std::stoi(value);
    // } catch (const std::exception& e) {
    //     throw ValueConversionException("Error converting value for id '"
    //         + this->getIniKey(id) + "' to integer: " + e.what());
    // }
}

std::set<std::string> Configuration::getList(const std::string& id, 
                                             const char delimiter) const {
    std::set<std::string> result;
    std::string value = get(id);
    
    size_t start = 0;
    size_t end = value.find(delimiter);
    while (end != std::string::npos) {
        result.insert(value.substr(start, end - start));
        start = end + 1;
        end = value.find(delimiter, start);
    }
    result.insert(value.substr(start, end));

    return result;
}

std::string Configuration::getThemePath() const {

    // TODO Do we really need to convert to int and then again to string?
    std::string themePath = 
        get(Configuration::THEME_PATH) 
        + std::to_string(getInt(Configuration::SCREEN_WIDTH)) 
        + "x" 
        + std::to_string(getInt(Configuration::SCREEN_HEIGHT)) 
        + "/" 
        + get(Configuration::THEME) 
        + "/";

    return themePath;
}

std::map<std::string, ConsoleData> Configuration::parseIniFile(const std::string& iniPath) {
    boost::property_tree::ptree pt;
    boost::property_tree::read_ini(iniPath, pt);

    std::map<std::string, ConsoleData> consoleDataMap;
    auto consoleList = pt.get<std::string>("CONSOLES.consoleList");
    std::stringstream ss(consoleList);
    std::string consoleName;

    while (std::getline(ss, consoleName, ',')) {
        ConsoleData data;
        data.name = consoleName;
        
        std::string execs_str = pt.get<std::string>(consoleName + ".execs");
        std::stringstream ss(execs_str);
        std::string exec;
        while (std::getline(ss, exec, ',')) {
            data.execs.push_back(exec);
        }
        std::string romExts_str = pt.get<std::string>(consoleName + ".romExts");
        ss = std::stringstream(romExts_str);
        std::string romExt;
        while (std::getline(ss, romExt, ',')) {
            data.romExts.push_back(romExt);
        }
        std::string romDirs_str = pt.get<std::string>(consoleName + ".romDirs");
        ss = std::stringstream(romDirs_str);
        std::string romDir;
        while (std::getline(ss, romDir, ',')) {
            data.romDirs.push_back(romDir);
        }

        consoleDataMap[consoleName] = data;
    }

    return consoleDataMap;
}

void Configuration::saveConfigIni() {
    
    boost::property_tree::ini_parser::write_ini(configIniFilepath, mainPt);

    std::cout << configIniFilepath << " written.\n";
}
