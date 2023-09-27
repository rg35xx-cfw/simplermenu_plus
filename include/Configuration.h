#pragma once
#include <boost/property_tree/ptree.hpp>
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

class Configuration {
private:

    std::string configIniFilepath;

    boost::property_tree::ptree mainPt;

public:

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
};
