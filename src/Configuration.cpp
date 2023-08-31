#include "Configuration.h"
#include "Exception.h"
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string.hpp>

Configuration::Configuration() {

    // initialize string to SettingID map
    std::unordered_map<SettingId, std::string>::iterator it = 
        this->idToStr.begin();
    while (it != this->idToStr.end()) {
        this->strToId[it->second] = it->first;
        ++it;
    }

    // Load values from .ini file using Boost.PropertyTree
    boost::property_tree::ptree pt;
    // FIXME: need to put relative path from $HOME for config.ini
    boost::property_tree::ini_parser::read_ini("/userdata/system/simplermenu_plus/config.ini", pt);

    for (const auto& section : pt) {
        for (const auto& key_value : section.second) {
            // TODO add all settings to the enum and map
            //      check before what happens with unknown values
            //      and fix it if needed
            SettingId full_key = this->strToId[section.first + "." + key_value.first];
            this->configValues[full_key] = key_value.second.get_value<std::string>();
        }
    }
}

Configuration& Configuration::getInstance() {
    static Configuration instance; // Guaranteed to be destroyed and instantiated on first use.
    return instance;
}

void Configuration::setValue(const SettingId& settingId, const std::string& value) {
    this->configValues[settingId] = value;
}

std::string Configuration::getValue(const SettingId& settingId) const {
    auto it = this->configValues.find(settingId);
    if (it != this->configValues.end()) {
        return it->second;
    } else {
        throw IniValueNotFoundException("Value not found for settingId: " 
            + this->getIniKey(settingId)); 
    }
}

bool Configuration::getBoolValue(const SettingId& settingId) const {
    std::string value = boost::algorithm::to_lower_copy(getValue(settingId));
    try {
        return value == "on";
    } catch (const std::exception& e) {
        throw ValueConversionException("Error converting value for settingId '"
            + this->getIniKey(settingId) + "' to boolean: " + e.what());
    }
}

int Configuration::getIntValue(const SettingId& settingId) const {
    std::string value = getValue(settingId);
    try {
        return std::stoi(value);
    } catch (const std::exception& e) {
        throw ValueConversionException("Error converting value for settingId '"
            + this->getIniKey(settingId) + "' to integer: " + e.what());
    }
}

std::set<std::string> Configuration::getStringList(const SettingId& settingId, 
                                                   char delimiter) const {
    std::set<std::string> result;
    std::string value = getValue(settingId);
    
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


    std::string themePath = 
        getValue(SettingId::THEME_PATH) 
        + std::to_string(getIntValue(SettingId::SCREEN_WIDTH)) 
        + "x" 
        + std::to_string(getIntValue(SettingId::SCREEN_HEIGHT)) 
        + "/" 
        + getValue(SettingId::THEME_NAME) 
        + "/";

    return themePath;
}

std::string Configuration::getIniKey(const SettingId& settingId) const {
    auto it = this->idToStr.find(settingId);
    if (it != this->idToStr.end()) {
        return it->second;
    } else {
        throw ItemIdNotFoundException("Value not found for key index: " 
                                      + settingId); 
    }
}

SettingId Configuration::getSettingId(const std::string& key) const {
    auto it = this->strToId.find(key);
    if (it != this->strToId.end()) {
        return it->second;
    } else {
        throw IniValueNotFoundException("Value not found for key: " + key); 
    }
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