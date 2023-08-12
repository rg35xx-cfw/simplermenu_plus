#pragma once
#include <string>
#include <unordered_map>
#include <set>

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
    int getIntValue(const std::string& key) const;
    std::set<std::string> getStringList(const std::string& key, char delimiter = ',') const;
    std::string getThemePath() const;
};
