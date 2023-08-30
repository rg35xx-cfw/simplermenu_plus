#include "Configuration.h"
#include "Exception.h"
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string.hpp>

Configuration::Configuration() {
    // Load values from .ini file using Boost.PropertyTree
    boost::property_tree::ptree pt;
    // FIXME: need to put relative path from $HOME for config.ini
    boost::property_tree::ini_parser::read_ini("/userdata/system/simplermenu_plus/config.ini", pt);

    for (const auto& section : pt) {
        for (const auto& key_value : section.second) {
            std::string full_key = section.first + "." + key_value.first;
            configValues[full_key] = key_value.second.get_value<std::string>();
        }
    }
}

Configuration& Configuration::getInstance() {
    static Configuration instance; // Guaranteed to be destroyed and instantiated on first use.
    return instance;
}

void Configuration::setValue(const std::string& key, const std::string& value) {
    configValues[key] = value;
}

std::string Configuration::getValue(const std::string& key) const {
    auto it = configValues.find(key);
    if (it != configValues.end()) {
        return it->second;
    } else {
        throw IniValueNotFoundException("Value not found for key: " + key); 
    }
}

bool Configuration::getBoolValue(const std::string& key) const {
    std::string value = boost::algorithm::to_lower_copy(getValue(key));
    try {
        return value == "on";
    } catch (const std::exception& e) {
        std::cerr << "Error converting value for key '" << key << "' to boolean: " << e.what() << std::endl;
        return false; // Return a default value or handle the error appropriately
    }
}

int Configuration::getIntValue(const std::string& key) const {
    std::string value = getValue(key);
    try {
        return std::stoi(value);
    } catch (const std::exception& e) {
        std::cerr << "Error converting value for key '" << key << "' to integer: " << e.what() << std::endl;
        return 0; // Return a default value or handle the error appropriately
    }
}

std::set<std::string> Configuration::getStringList(const std::string& key, char delimiter) const {
    std::set<std::string> result;
    std::string value = getValue(key);
    
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


    std::string themePath = getValue("Menu.themePath") + std::to_string(getIntValue("Menu.screenWidth")) +
                            "x" + std::to_string(getIntValue("Menu.screenHeight")) + "/" + getValue("Menu.themeName") + "/";
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