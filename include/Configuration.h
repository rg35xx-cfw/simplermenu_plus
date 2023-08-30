#pragma once
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

class Configuration {
private:
    std::unordered_map<std::string, std::string> configValues;
    Configuration(); // Private constructor for singleton

public:
    // Delete copy constructor and assignment operator
    Configuration(const Configuration&) = delete;
    Configuration& operator=(const Configuration&) = delete;

    // Static method to get the instance
    static Configuration& getInstance();

    void setValue(const std::string& key, const std::string& value);
    std::string getValue(const std::string& key) const;
    bool getBoolValue(const std::string& key) const;
    int getIntValue(const std::string& key) const;
    std::set<std::string> getStringList(const std::string& key, char delimiter = ',') const;
    std::string getThemePath() const;
    std::map<std::string, ConsoleData> parseIniFile(const std::string& iniPath);

};
