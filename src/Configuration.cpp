#include "Configuration.h"
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

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
        return "NOT FOUND"; 
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
