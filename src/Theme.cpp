#include "Theme.h"
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string.hpp>

Theme::Theme() {
    // Load values from .ini file using Boost.PropertyTree
    boost::property_tree::ptree pt;

    // FIXME: Removed hardcoded theme.ini 
    baseThemePath = "/userdata/system/.simplemenu/themes/640x480/Simplermenu/";
    boost::property_tree::ini_parser::read_ini(baseThemePath + "theme.ini", pt);

    for (const auto& section : pt) {
        for (const auto& key_value : section.second) {
            std::string full_key = section.first + "." + key_value.first;
            configValues[full_key] = key_value.second.get_value<std::string>();
        }
    }
}

Theme& Theme::getInstance() {
    static Theme instance; // Guaranteed to be destroyed and instantiated on first use.
    return instance;
}

void Theme::setValue(const std::string& key, const std::string& value) {
    configValues[key] = value;
}

std::string Theme::getValue(const std::string& key, bool relative) const {
    auto it = configValues.find(key);
    if (it != configValues.end()) {
        return relative ? baseThemePath + it->second : it->second;
    } else {
        // TODO fire an exception?
        return "NOT FOUND"; 
    }
}

bool Theme::getBoolValue(const std::string& key) const {
    std::string value = boost::algorithm::to_lower_copy(getValue(key));
    try {
        return value == "on";
    } catch (const std::exception& e) {
        std::cerr << "Error converting value for key '" << key << "' to boolean: " << e.what() << std::endl;
        return false; // Return a default value or handle the error appropriately
    }
}

int Theme::getIntValue(const std::string& key) const {
    std::string value = getValue(key);
    try {
        return std::stoi(value);
    } catch (const std::exception& e) {
        std::cerr << "Error converting value for key '" << key << "' to integer: " << e.what() << std::endl;
        return 0; // Return a default value or handle the error appropriately
    }
}

std::set<std::string> Theme::getStringList(const std::string& key, char delimiter) const {
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

SDL_Color Theme::getColor(const std::string& key) const {
    std::string value = getValue(key);
    unsigned long valColor = stoul(value, nullptr, 16);

    SDL_Color color;

    color.r = (valColor >> 16) & 0xff;
    color.g = (valColor >> 8) & 0xff;
    color.b = (valColor >> 0) & 0xff;

    return color;
}

std::string Theme::getThemePath() const {


    std::string themePath = getValue("Menu.themePath") + std::to_string(getIntValue("Menu.screenWidth")) +
                            "x" + std::to_string(getIntValue("Menu.screenHeight")) + "/" + getValue("Menu.themeName") + "/";
    return themePath;
}
