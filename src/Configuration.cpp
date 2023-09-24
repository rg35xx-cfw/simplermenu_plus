#include "Configuration.h"
#include "Exception.h"
#include <iostream>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string.hpp>


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
        get("MENU.themePath") 
        + std::to_string(getInt("MENU.screenWidth")) 
        + "x" 
        + std::to_string(getInt("MENU.screenHeight")) 
        + "/" 
        + get("MENU.themeName") 
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
