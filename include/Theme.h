#pragma once
#include <string>
#include <unordered_map>
#include <set>

class Theme {
private:
    std::unordered_map<std::string, std::string> configValues;
    std::string baseThemePath;
    Theme(); // Private constructor for singleton

public:
    // Delete copy constructor and assignment operator
    Theme(const Theme&) = delete;
    Theme& operator=(const Theme&) = delete;

    // Static method to get the instance
    static Theme& getInstance();

    void setValue(const std::string& key, const std::string& value);
    std::string getValue(const std::string& key, bool relative = false) const;
    bool getBoolValue(const std::string& key) const;
    int getIntValue(const std::string& key) const;
    std::set<std::string> getStringList(const std::string& key, char delimiter = ',') const;
    std::string getThemePath() const;
};
