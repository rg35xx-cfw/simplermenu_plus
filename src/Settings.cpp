#include <iostream>
#include <filesystem>
#include <SDL/SDL.h>

#include <boost/algorithm/string.hpp>

#include "Configuration.h"
#include "Theme.h"
#include "I18n.h"
#include "Settings.h"
#include "Exception.h"

Settings::Settings(Configuration& cfg, I18n& i18n, 
                   ISettingsObserver *observer, ILanguageSubject *langSubject) 
    : cfg(cfg), i18n(i18n) {
}

SystemSettings::SystemSettings(Configuration& cfg, I18n& i18n, 
                               ISettingsObserver *observer, 
                               ILanguageSubject *langSubject)
    : Settings(cfg, i18n, observer, langSubject) {
    defaultKeys = {
        Configuration::VOLUME, Configuration::BRIGHTNESS, Configuration::SCREEN_REFRESH,
        Configuration::SHOW_FPS, Configuration::OVERCLOCK, Configuration::THEME,
        Configuration::USB_MODE, Configuration::WIFI, Configuration::ROTATION,
        Configuration::LANGUAGE,
        Configuration::UPDATE_CACHES, Configuration::SAVE_SETTINGS, Configuration::RESTART, 
        Configuration::QUIT
    };

    // First attach observers in to let them know about initial values
    // TODO should the observer attach itself instead?
    attach(observer);
    langSubject->attach(this);

    // Now we can initialize the settings and the observers will receive
    // the values
    initializeSettings();

    // enabledKeys = getEnabledKeys();
}

FolderSettings::FolderSettings(Configuration& cfg, I18n& i18n, 
                               ISettingsObserver *observer, 
                               ILanguageSubject *langSubject)
    : Settings(cfg, i18n, observer, langSubject) {
    defaultKeys = {
        Configuration::CORE_SELECTION
    };

    // First attach observers in to let them know about initial values
    // TODO should the observer attach itself instead?
    attach(observer);
    langSubject->attach(this);

    // Now we can initialize the settings and the observers will receive
    // the values
    initializeSettings();

    // enabledKeys = getEnabledKeys();
}

 RomSettings::RomSettings(Configuration& cfg, I18n& i18n, 
                          ISettingsObserver *observer, 
                          ILanguageSubject *langSubject)
        : Settings(cfg, i18n, observer, langSubject) {
    defaultKeys = {
        Configuration::ROM_OVERCLOCK, Configuration::ROM_AUTOSTART, Configuration::CORE_OVERRIDE
    };    

    // First attach observers in to let them know about initial values
    // TODO should the observer attach itself instead?
    attach(observer);
    langSubject->attach(this);

    // Now we can initialize the settings and the observers will receive
    // the values
    initializeSettings();

    // enabledKeys = getEnabledKeys();
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
            updateBrightness(false); // false for left/decrease direction
        } else if (currentKey == Configuration::VOLUME) {
            updateVolume(false);
        } else if (currentKey == Configuration::SCREEN_REFRESH) {
            updateScreenRefresh(false);
        } else if (currentKey == Configuration::THEME) {
            updateTheme(false);
        } else if (currentKey == Configuration::USB_MODE) {
            updateUSBMode(false);
        } else if (currentKey == Configuration::OVERCLOCK) {
            updateOverclock(true);
        } else if (currentKey == Configuration::SHOW_FPS) {
            updateShowFPS();
        } else if (currentKey == Configuration::LANGUAGE) {
            updateLanguage(false);
        } else if (currentKey == Configuration::CORE_OVERRIDE) {
            updateCoreOverride(false);
        } else if (currentKey == Configuration::CORE_SELECTION) {
            updateCoreSelection(false);
        }  
    }
    notifySettingsChange(currentKey, currentValue);
}

void Settings::navigateRight() {
    std::cout << "navigate Right" << std::endl;
    if (settingsMap[currentKey].enabled) {
        if (currentKey == Configuration::BRIGHTNESS) {
            updateBrightness(true); // true for right/increase direction
        } else if (currentKey == Configuration::VOLUME) {
            updateVolume(true);
        } else if (currentKey == Configuration::SCREEN_REFRESH) {
            updateScreenRefresh(true);
        } else if (currentKey == Configuration::THEME) {
            updateTheme(true);
        } else if (currentKey == Configuration::USB_MODE) {
            updateUSBMode(false);
        } else if (currentKey == Configuration::OVERCLOCK) {
            updateOverclock(false);
        } else if (currentKey == Configuration::SHOW_FPS) {
            updateShowFPS();
        } else if (currentKey == Configuration::LANGUAGE) {
            updateLanguage(true);
        } else if (currentKey == Configuration::CORE_OVERRIDE) {
            updateCoreOverride(true);
        } else if (currentKey == Configuration::CORE_SELECTION) {
            updateCoreSelection(true);
        }     
    }
    notifySettingsChange(currentKey, currentValue);
}

void Settings::navigateEnter() {
    std::cout << "navigate Enter" << std::endl;
    if (settingsMap[currentKey].enabled) {
        if (currentKey == Configuration::SAVE_SETTINGS) {
            saveSettings();
        } else if (currentKey == Configuration::RESTART) {
            restartApplication();
        } else if (currentKey == Configuration::QUIT) {
            quitApplication();
        }
    }

}

std::vector<Settings::I18nSetting> SystemSettings::getSystemSettings() {
    return i18nSettings;
}

std::vector<Settings::I18nSetting> FolderSettings::getFolderSettings() {
    
    std::vector<I18nSetting> i18nSettings;
    
    for (const auto& key : enabledKeys) {

        if (key.find("FOLDER.") == 0) {
        
            size_t pos = key.find_last_of(".");
            
            if (pos != std::string::npos) {
                try {
                    i18nSettings.push_back({i18n.get(key.substr(pos + 1)), 
                                            settingsMap[key].value
                                            });
                } catch (boost::property_tree::ptree_bad_path e) {
                    throw ItemNotFoundException("Language translation not found for " 
                    + key + " in " + i18n.getLang());
                }
            } else {
                throw ItemNotFoundException("Setting key format unknown: " 
                    + key);
            }
        }
    }

    return i18nSettings;
}

std::vector<Settings::I18nSetting> RomSettings::getRomSettings() {
    
    std::vector<I18nSetting> i18nSettings;
    
    for (const auto& key : enabledKeys) {

        if (key.find("GAME.") == 0) {
        
            size_t pos = key.find_last_of(".");
            
            if (pos != std::string::npos) {
                try {
                    i18nSettings.push_back({i18n.get(key.substr(pos + 1)), 
                                            settingsMap[key].value
                                            });
                } catch (boost::property_tree::ptree_bad_path e) {
                    throw ItemNotFoundException("Language translation not found for " 
                    + key + " in " + i18n.getLang());
                }
            } else {
                throw ItemNotFoundException("Setting key format unknown: " 
                    + key);
            }
        }
    }

    return i18nSettings;
}


void Settings::initializeSettings() {

    std::string themePath = 
        cfg.get(Configuration::THEME_PATH) 
        + cfg.get(Configuration::SCREEN_WIDTH) + "x" 
        + cfg.get(Configuration::SCREEN_HEIGHT) + "/";

    for (const auto& entry : std::filesystem::directory_iterator(themePath)) {
        if (entry.is_directory()) {
                themeFolders.insert(entry.path().filename().string());
        }
    }
    
    for (const auto& theme: themeFolders) {
        std::cout << "theme: " << theme << std::endl;
    }

    for (const auto& key : defaultKeys) {
        std::string value = cfg.get(key);
        if (!value.empty()) {
            settingsMap[key] = {key, value, true}; // enabled
            notifySettingsChange(key, value);
        } else {
            settingsMap[key] = {key, "", false}; // disabled
        }
    }

    enabledKeys = getEnabledKeys();

    currentIndex = 0;
    if(!enabledKeys.empty()) {
        currentKey = enabledKeys[currentIndex];
    }

    // First time we need to initialize the language manually
    // as it needs the complete list of initialized settings
    // to generate the internationalized version of them
    languageChanged();
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

void Settings::updateBrightness(bool increase) {
    int currentValue = std::stoi(settingsMap[Configuration::BRIGHTNESS].value);
    if (increase) {
        currentValue += 10; // decrease brightness by 10 units
    } else {
        currentValue -= 10; // increase brightness by 10 units
    }
    settingsMap[Configuration::BRIGHTNESS].value = std::to_string(currentValue);
    std::cout << "brightness: " << settingsMap[Configuration::BRIGHTNESS].value << std::endl;
    // FIXME: add boundaries (e.g. min: 0, max: 100)
}

void Settings::updateVolume(bool increase) {
    int currentValue = std::stoi(settingsMap[Configuration::VOLUME].value);
    if (increase) {
        currentValue += 5; // decrease volume by 5 units
    } else {
        currentValue -= 5; // increase volume by 5 units
    }
    settingsMap[Configuration::VOLUME].value = std::to_string(currentValue);
    // FIXME: add boundaries (e.g. min: 0, max: 100)
}

void Settings::updateScreenRefresh(bool increase) {
    int currentValue = std::stoi(settingsMap[Configuration::SCREEN_REFRESH].value);
    if (increase) {
        currentValue += 5; // decrease Screen Refresh by 5 units
    } else {
        currentValue -= 5; // increase Screen Refresh by 5 units
    }
    settingsMap[Configuration::SCREEN_REFRESH].value = std::to_string(currentValue);
    // FIXME: add boundaries (e.g. min: 0, max: 100)
}

void Settings::updateListSetting(const std::set<std::string>& values, bool increase) {
    auto it = values.find(currentValue);
    
    if (!increase) {
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
}

void Settings::updateTheme(bool increase) {
    updateListSetting(themeFolders, increase);
    
    settingsMap[Configuration::THEME].value = currentValue;

    std::cout << "UPDATING THEME" << std::endl;
}

void Settings::updateUSBMode(bool increase) {
    updateListSetting(cfg.getList(Configuration::USB_MODE_VALUES), increase);

    settingsMap[Configuration::USB_MODE].value = currentValue;

    std::cout << "UPDATING USB MODE" << std::endl;
}

void Settings::updateLanguage(bool increase) {
    // First we need to normalize the value
    currentValue = boost::algorithm::to_upper_copy(settingsMap[currentKey].value);
    std::cout << "CURRENT LANGUAGE " << currentValue << std::endl;

    updateListSetting(i18n.getLanguages(), increase);

    std::cout << "UPDATING LANGUAGE TO " << currentValue << std::endl;

    settingsMap[currentKey].value = currentValue;
}

void Settings::updateOverclock(bool increase) {
    updateListSetting(cfg.getList(Configuration::OVERCLOCK_VALUES), increase);

    settingsMap[Configuration::OVERCLOCK].value = currentValue;

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

void Settings::updateWifi() {
    updateBoolSetting();
    std::cout << "UPDATING Wifi" << std::endl;
}

void Settings::updateRotation() {
    updateBoolSetting();
    std::cout << "UPDATING Rotation" << std::endl;
}

void Settings::saveSettings() {
    std::cout << "SAVING SETTINGS..." << std::endl;
}

void Settings::restartApplication() {
    std::cout << "RESTART..." << std::endl;
}

void Settings::quitApplication() {
    std::cout << "QUIT..." << std::endl;
    SDL_Quit();
    exit(0);
}

void Settings::updateCoreSelection(bool increase) {
    updateListSetting(cores, increase);

    settingsMap[Configuration::CORE_SELECTION].value = currentValue;

    std::cout << "UPDATING CORE SELECTION" << std::endl;
}

void Settings::updateCoreOverride(bool increase) {
    updateListSetting(cores, increase);

    settingsMap[Configuration::CORE_OVERRIDE].value = currentValue;

    std::cout << "UPDATING CORE OVERRIDE" << std::endl;
}

std::string Settings::getCurrentKey() {
    return currentKey;
};

std::string Settings::getCurrentValue() {
    return settingsMap[currentKey].value;
};

////////////
// Methods to manage SETTINGS OBSERVERS

void Settings::attach(ISettingsObserver *observer) {
    observers.push_back(observer);
    std::cout << "Observer added to " << getName() << " object: " << observer->getName() << "\n";
}

void Settings::detach(ISettingsObserver *observer) {
    observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
}

void Settings::notifySettingsChange(const std::string &key, const std::string &value) {
    for (ISettingsObserver *observer : observers) {
        observer->settingsChanged(key, value);
        std::cout << "Observer " << observer->getName() << " notified by " 
                  << getName() << std::endl;
    }
    reloadI18nSettings();
}

void Settings::reloadI18nSettings() {
    i18nSettings.clear();

    for (const auto& key : enabledKeys) {

        std::cout << key << std::endl;
        
        if (key.find("SYSTEM.") == 0) {

            size_t pos = key.find_last_of(".");
            
            if (pos != std::string::npos) {
                try {
                    i18nSettings.push_back({i18n.get(key.substr(pos + 1)), 
                                            settingsMap[key].value
                                            });
                } catch (boost::property_tree::ptree_bad_path e) {
                    throw ItemNotFoundException("Language translation not found for " 
                    + key + " in " + i18n.getLang());
                }
            } else {
                throw ItemNotFoundException("Setting key format unknown: " 
                    + key);
            }
        }
    }
}

/////////
// ILanguageObserver methods

void SystemSettings::languageChanged() {
    std::cout << "Reloading system settings labels due to language change" 
            << std::endl;

    reloadI18nSettings();    
}

void RomSettings::languageChanged() {

    // TODO not implemented
}

void FolderSettings::languageChanged() {

    // TODO not implemented

}



/////////////////
// ISettingsObserver and ILanguageSubject common methods

std::string SystemSettings::getName() {
    return "SystemSettings::" + std::to_string((unsigned long long)(void**)this);
}

std::string RomSettings::getName() {
    return "RomSettings::" + std::to_string((unsigned long long)(void**)this);
}

std::string FolderSettings::getName() {
    return "FolderSettings::" + std::to_string((unsigned long long)(void**)this);
}

//
/////////////