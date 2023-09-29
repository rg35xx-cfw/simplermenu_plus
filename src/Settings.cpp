#include <iostream>

#include "Settings.h"
#include "Configuration.h"

Settings::Settings(Configuration& cfg) : cfg(cfg) {
    defaultKeys = {
        Configuration::VOLUME, Configuration::BRIGHTNESS, Configuration::SCREEN_REFRESH,
        Configuration::SHOW_FPS, Configuration::OVERCLOCK, Configuration::THEME,
        Configuration::USB_MODE, Configuration::WIFI, Configuration::ROTATION
    };

    initializeSettings();

    enabledKeys = getEnabledKeys();
    currentIndex = 0;
    if(!enabledKeys.empty()) {
        currentKey = enabledKeys[currentIndex];
    }
}

void Settings::navigateUp() {
    std::cout << "navigate Up" << std::endl;
     if (!enabledKeys.empty()) {
        currentIndex--;
        if (currentIndex < 0) {
            currentIndex = enabledKeys.size() - 1; // wrap around to the end
        }
        currentKey = enabledKeys[currentIndex];
    }
}

void Settings::navigateDown() {
    std::cout << "navigate Down" << std::endl;
    if (!enabledKeys.empty()) {
        currentIndex++;
        if (currentIndex >= enabledKeys.size()) {
            currentIndex = 0; // wrap around to the start
        }
        currentKey = enabledKeys[currentIndex];
    }
}

void Settings::navigateLeft() {
    std::cout << "navigate Left" << std::endl;
    if (settingsMap[currentKey].enabled) {
        if (currentKey == Configuration::BRIGHTNESS) {
            updateBrightness(true); // true for left direction
        } else if (currentKey == Configuration::VOLUME) {
            updateVolume(true);
        } else if (currentKey == Configuration::SCREEN_REFRESH) {
            updateScreenRefresh(true);
        } else if (currentKey == Configuration::THEME) {
            updateTheme(true);
        } else if (currentKey == Configuration::OVERCLOCK) {
            updateOverclock(true);
        } else if (currentKey == Configuration::SHOW_FPS) {
            updateShowFPS();
        }    
    }

}

void Settings::navigateRight() {
    std::cout << "navigate Right" << std::endl;
        if (settingsMap[currentKey].enabled) {
        if (currentKey == Configuration::BRIGHTNESS) {
            updateBrightness(false); // true for left direction
        } else if (currentKey == Configuration::VOLUME) {
            updateVolume(false);
        } else if (currentKey == Configuration::SCREEN_REFRESH) {
            updateScreenRefresh(false);
        } else if (currentKey == Configuration::THEME) {
            updateTheme(false);
        } else if (currentKey == Configuration::OVERCLOCK) {
            updateOverclock(false);
        } else if (currentKey == Configuration::SHOW_FPS) {
            updateShowFPS();
        }   
    }
}

void Settings::navigateEnter() {
    std::cout << "navigate Enter" << std::endl;

}

void Settings::initializeSettings() {
    for (const auto& key : defaultKeys) {
        std::string value = cfg.get(key);
        if (!value.empty()) {
            settingsMap[key] = {key, cfg.get(key), true}; // enabled
        } else {
            settingsMap[key] = {key, "", false}; // disabled
        }
    }
}

std::vector<std::string> Settings::getEnabledKeys() {
    std::vector<std::string> enabledKeys;
    for (const auto& key : defaultKeys) {
        if (settingsMap[key].enabled) {
            enabledKeys.push_back(key);
        }
    }

    return enabledKeys;
}

void Settings::updateBrightness(bool isLeft) {
    int currentValue = std::stoi(settingsMap[Configuration::BRIGHTNESS].value);
    if (isLeft) {
        currentValue -= 10; // decrease brightness by 10 units
    } else {
        currentValue += 10; // increase brightness by 10 units
    }
    settingsMap[Configuration::BRIGHTNESS].value = std::to_string(currentValue);
    std::cout << "brightness: " << settingsMap[Configuration::BRIGHTNESS].value << std::endl;
    // FIXME: add boundaries (e.g. min: 0, max: 100)
}

void Settings::updateVolume(bool isLeft) {
    int currentValue = std::stoi(settingsMap[Configuration::VOLUME].value);
    if (isLeft) {
        currentValue -= 5; // decrease volume by 5 units
    } else {
        currentValue += 5; // increase volume by 5 units
    }
    settingsMap[Configuration::VOLUME].value = std::to_string(currentValue);
    // FIXME: add boundaries (e.g. min: 0, max: 100)
}

void Settings::updateScreenRefresh(bool isLeft) {
    int currentValue = std::stoi(settingsMap[Configuration::SCREEN_REFRESH].value);
    if (isLeft) {
        currentValue -= 5; // decrease Screen Refresh by 5 units
    } else {
        currentValue += 5; // increase Screen Refresh by 5 units
    }
    settingsMap[Configuration::SCREEN_REFRESH].value = std::to_string(currentValue);
    // FIXME: add boundaries (e.g. min: 0, max: 100)
}

void Settings::updateListSetting(bool isLeft) {
    std::set<std::string> values = cfg.getList(currentKey);
    auto it = values.find(currentValue);
    
    if (isLeft) {
    if (it == values.begin()) {
            it = std::prev(values.end());
        } else {
            --it;
        }
    } else {
        ++it;
        if (it == values.end()) {
            it = values.begin();
        }
    }
    currentValue = *it;
    settingsMap[currentKey].value = currentValue;
}

void Settings::updateTheme(bool isLeft) {
    updateListSetting(isLeft);

    std::cout << "UPDATING THEME" << std::endl;
}

void Settings::updateOverclock(bool isLeft) {
    updateListSetting(isLeft);

    std::cout << "UPDATING OVERCLOCK" << std::endl;
}

void Settings::updateBoolSetting() {
    bool value = settingsMap[currentKey].value == "true"; 
    value = !value; // Toggle the value
    currentValue = value ? "true" : "false";
    settingsMap[currentKey].value = currentValue;
}

void Settings::updateShowFPS() {
    updateBoolSetting();

    std::cout << "UPDATING FPS SHOW" << std::endl;
}

