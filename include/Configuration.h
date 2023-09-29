#pragma once
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include <map>
#include <set>
#include <vector>
#include <memory>
#include "Settings.h"

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

    boost::property_tree::ptree mainPt;

public:

    static const std::string SEL_ITEM_FONT_COLOR;
    static const std::string ITEMS_FONT_COLOR;


    Configuration(const std::string& configIniFilepath);

    void set(const std::string& id, const std::string& value);

    std::string get(const std::string& id) const;
    bool getBool(const std::string& id) const;
    int getInt(const std::string& id) const;

    std::set<std::string> getList(const std::string& id, 
                                  const char delimiter = ',') const;
    std::string getThemePath() const;
    std::map<std::string, ConsoleData> parseIniFile(const std::string& iniPath);

    void saveConfigIni();

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
