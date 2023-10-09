#pragma once
#include <string>
#include <unordered_map>
#include <set>
#include <SDL/SDL.h>

#include "Configuration.h"

class Theme {
private:
    std::unordered_map<std::string, std::string> configValues;
    std::string baseThemePath;

public:
    Theme(std::string name, int screenWidth, int screenHeight);

    void loadTheme(const std::string& themeName, int screenWidth, int screenHeight);
    void setValue(const std::string& key, const std::string& value);
    std::string getValue(const std::string& key, bool relative = false) const;
    bool getBoolValue(const std::string& key) const;
    int getIntValue(const std::string& key) const;
    std::set<std::string> getStringList(const std::string& key, char delimiter = ',') const;
    SDL_Color getColor(const std::string& key) const;
    std::string getThemePath() const;
};
