#pragma once

#include <string>
#include <map>
#include <vector>
#include "I18n.h"
#include "IObservers.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>

class Configuration;

class Settings : public ISettingsSubject {
public: 
    struct Setting {
        std::string key;
        std::string value;
        bool enabled;
    };

    struct I18nSetting {
        std::string title;
        std::string value;
    };

    virtual void navigateUp();
    virtual void navigateDown();
    virtual void navigateLeft() = 0;
    virtual void navigateRight() = 0;
    virtual void navigateEnter() = 0;

private:
    std::vector<ISettingsObserver *> observers;
    
protected:
    int currentIndex;

    std::string currentKey;
    std::string currentValue;
    
    std::set<std::string> themeFolders;

    Configuration& cfg;
    I18n& i18n;
    std::vector<I18nSetting> i18nSettings;

    std::vector<std::string> enabledKeys;

    std::map<std::string, Setting> settingsMap;

    int minValue;
    int maxValue;
    int delta;

    void updateInt(bool increase, std::string setting, 
                   int min, int max, int inc);
    
    std::set<std::string> cores;

    std::vector<std::string> getEnabledKeys();

    /**
     * ISettingsSubject methods
     */
    void notifySettingsChange(const std::string &key, const std::string &value) override;


public:
    Settings(Configuration& cfg, I18n& i18n, 
             int minValue, int maxValue, int delta);
    // ~Settings();

    // Define default settings with their keys
    std::vector<std::string> defaultKeys;

    void initializeSettings();

    void updateListSetting(const std::set<std::string>& values, bool increase);
    void updateBoolSetting();

    std::string getCurrentKey();
    std::string getCurrentValue();

    /**
     * ISettingsSubject methods
    */
    void attach(ISettingsObserver *observer) override;
    void detach(ISettingsObserver *observer) override;
    
    /**
     * ISettingsSubject and ILanguageObserver common methods
     */
    virtual std::string getName() = 0;
};

class AppSettings : public Settings {
public:

    AppSettings(Configuration& cfg, I18n& i18n, 
                   int minValue, int maxValue, int delta);

    std::vector<Settings::I18nSetting> getAppSettings();

    void reloadI18nSettings();

    // these methods should just call the parent class's methods. Is it possible to defined them as pure virtual?
    void navigateUp() { Settings::navigateUp(); };
    void navigateDown() { Settings::navigateDown();};
    void navigateLeft() override {
        std::cout << "navigate Left" << std::endl;
        if (settingsMap[currentKey].enabled) {
            if (currentKey == Configuration::BRIGHTNESS) {
                updateInt(false, currentKey, minValue, maxValue, delta);
            
            } else if (currentKey == Configuration::VOLUME) {
                updateInt(false, currentKey, minValue, maxValue, delta);
            
            } else if (currentKey == Configuration::SCREEN_REFRESH) {
                updateInt(false, currentKey, minValue + delta, maxValue, delta);
            
            } else if (currentKey == Configuration::THEME) {
                updateTheme(false);
            
            } else if (currentKey == Configuration::THUMBNAIL_TYPE) {
                updateThumbnailType(false);
            
            } else if (currentKey == Configuration::WIFI) {
                updateWifi();
            
            } else if (currentKey == Configuration::ROTATION) {
                updateRotation();

            } else if (currentKey == Configuration::USB_MODE) {
                updateUSBMode(false);
            
            } else if (currentKey == Configuration::OVERCLOCK) {
                updateOverclock(true);
            
            } else if (currentKey == Configuration::SHOW_FPS) {
                updateShowFPS();
            
            } else if (currentKey == Configuration::LANGUAGE) {
                updateLanguage(false);
            
            }
        }
        notifySettingsChange(currentKey, currentValue);
    }

    void navigateRight() override {
        std::cout << "navigate Right" << std::endl;
        if (settingsMap[currentKey].enabled) {
            if (currentKey == Configuration::BRIGHTNESS) {
                updateInt(true, currentKey, minValue, maxValue, delta);

            } else if (currentKey == Configuration::VOLUME) {
                updateInt(true, currentKey, minValue, maxValue, delta);

            } else if (currentKey == Configuration::SCREEN_REFRESH) {
                updateInt(true, currentKey, minValue + delta, maxValue, delta);

            } else if (currentKey == Configuration::THEME) {
                updateTheme(true);

            } else if (currentKey == Configuration::THUMBNAIL_TYPE) {
                updateThumbnailType(true);

            } else if (currentKey == Configuration::WIFI) {
                updateWifi();

            } else if (currentKey == Configuration::ROTATION) {
                updateRotation();

            } else if (currentKey == Configuration::USB_MODE) {
                updateUSBMode(false);

            } else if (currentKey == Configuration::OVERCLOCK) {
                updateOverclock(false);

            } else if (currentKey == Configuration::SHOW_FPS) {
                updateShowFPS();

            } else if (currentKey == Configuration::LANGUAGE) {
                updateLanguage(true);

            }  
        }
        notifySettingsChange(currentKey, currentValue);
    }


    void navigateEnter() override {
        std::cout << "navigate Enter" << std::endl;
        // TODO this should notify the observers that the current setting
        //      has been changed
        if (settingsMap[currentKey].enabled) {
            if (currentKey == Configuration::RESTART) {
                restartApplication();
            } else if (currentKey == Configuration::QUIT) {

                quitApplication();
            }
        }
    }

    void updateTheme(bool increase);
    void updateThumbnailType(bool increase);
    void updateUSBMode(bool increase);
    void updateLanguage(bool increase);
    void updateOverclock(bool increase);
    void updateShowFPS();
    void restartApplication();
    void quitApplication();
    void updateWifi();
    void updateRotation();

    /**
     * ISettingsSubject methods 
     */
    std::string getName() override;
};


class SystemSettings : public Settings {
public:

    SystemSettings(Configuration& cfg, I18n& i18n, 
                   int minValue, int maxValue, int delta);

    std::vector<Settings::I18nSetting> getSystemSettings();

    void navigateUp() { Settings::navigateUp(); };
    void navigateDown() { Settings::navigateDown();};
    void navigateLeft() override {};
    void navigateRight() override {};
    void navigateEnter() override {};

    /**
     * ISettingsSubject methods 
     */
    std::string getName() override;

    void generateCoreSettings() {
        namespace fs = boost::filesystem;
        namespace pt = boost::property_tree;

        std::string sectionGroupsPath = cfg.get(Configuration::HOME_PATH) + "/section_groups/";
        pt::ptree root;

        // Iterate over all .ini files in the section_groups directory
        fs::directory_iterator end_itr; // default construction yields past-the-end
        for (fs::directory_iterator itr(sectionGroupsPath); itr != end_itr; ++itr) {
            if (fs::is_regular_file(itr->status()) && itr->path().extension() == ".ini") {
                std::string iniFilePath = itr->path().string();

                // Parse the ini file for folder names and their ConsoleData
                std::map<std::string, ConsoleData> consoleDataMap = cfg.parseIniFile(iniFilePath);

                for (const auto& pair : consoleDataMap) {
                    const std::string& folderName = pair.first;
                    const ConsoleData& consoleData = pair.second;

                    if (!consoleData.execs.empty()) {
                        pt::ptree folderData, coresArray;
                        std::string defaultCore;

                        for (const auto& exec : consoleData.execs) {
                            std::string coreName = exec.substr(exec.find_last_of("/\\") + 1);
                            coresArray.push_back(std::make_pair("", pt::ptree(coreName)));

                            if (defaultCore.empty()) {
                                defaultCore = coreName;
                            }
                        }

                        folderData.add_child("cores", coresArray);
                        folderData.put("default_core", defaultCore);

                        // Add to the root node under the folderName
                        root.add_child(folderName, folderData);
                    }
                }
            }
        }

        // Write to single JSON file
        std::string jsonFilePath = cfg.get(Configuration::HOME_PATH) + "coreSettings.json";
        pt::write_json(jsonFilePath, root);
    }

};

class RomSettings : public Settings, public ILanguageObserver {
public:
    RomSettings(Configuration& cfg, I18n& i18n, 
                int minValue, int maxValue, int delta);

    std::vector<Settings::I18nSetting> getRomSettings();

    void updateRomOverclock(bool increase);
    void updateAutoStart(bool increase);
    void updateCoreSelection(bool increase);
    void updateCoreOverride(bool increase);

    void navigateUp() { Settings::navigateUp(); };
    void navigateDown() { Settings::navigateDown();};
    void navigateEnter() override {
        std::cout << "RomSettings navigate Enter" << std::endl;
    };
    void navigateLeft() override {
        std::cout << "navigate Left" << std::endl;
        if (settingsMap[currentKey].enabled) {
            if (currentKey == Configuration::CORE_OVERRIDE) {
                updateCoreOverride(false);
            } else if (currentKey == Configuration::ROM_OVERCLOCK) {
                updateRomOverclock(false); 
            } else if (currentKey == Configuration::ROM_AUTOSTART) {
                updateAutoStart(false);            
            } /*else if (currentKey == Configuration::CORE_SELECTION) {
                updateCoreSelection(false);
            }  */
        }
        notifySettingsChange(currentKey, currentValue);
    }

    void navigateRight() override {
        std::cout << "navigate Right" << std::endl;
        if (settingsMap[currentKey].enabled) {
            if (currentKey == Configuration::CORE_OVERRIDE) {
                updateCoreOverride(true);
            } else if (currentKey == Configuration::ROM_OVERCLOCK) {
                updateRomOverclock(true); 
            } else if (currentKey == Configuration::ROM_AUTOSTART) {
                updateAutoStart(true);         
            } /*else if (currentKey == Configuration::CORE_SELECTION) {
                updateCoreSelection(true);
            }  */
        }
        notifySettingsChange(currentKey, currentValue);
    }
    /**
     * ILanguageObserver methods
     */
    void languageChanged() override;

    /**
     * ISettingsSubject methods 
     */
    std::string getName() override;

public:
    void getCores(std::string sectionName, std::string folderName) {

        std::map<std::string, ConsoleData> consoleDataMap = cfg.parseIniFile(cfg.get(Configuration::HOME_PATH) + "/section_groups/" + sectionName);

        cores.clear();

        // Check if the parentTitle exists in the consoleDataMap
        if (consoleDataMap.find(folderName) != consoleDataMap.end()) {
            // Access the ConsoleData for the parentTitle
            ConsoleData consoleData = consoleDataMap[folderName];

            // Check if the execs vector is not empty
            if (!consoleData.execs.empty()) {
                for(auto exec: consoleData.execs) {
                    cores.insert(exec.substr(exec.find_last_of("/\\") + 1));
                }
            }
        }

        // By default we select the first core from the list
        // TODO: need to add the logic to override a core/launcher per rom
        std::string currentCore = *cores.begin();
        settingsMap[Configuration::CORE_OVERRIDE] = {Configuration::CORE_OVERRIDE, currentCore, true};
        notifySettingsChange(Configuration::CORE_OVERRIDE, currentCore);
    }
};


