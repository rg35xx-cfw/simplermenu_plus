#pragma once

#include <string>
#include <map>
#include <vector>
#include "I18n.h"
#include "IObservers.h"

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

private:
    std::string currentKey;
    std::string currentValue;

    int currentIndex;

    std::vector<ISettingsObserver *> observers;

    std::set<std::string> themeFolders;


protected:
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

    void reloadI18nSettings();

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

    void navigateUp();
    void navigateDown();
    void navigateLeft();
    void navigateRight();
    void navigateEnter();

    void updateListSetting(const std::set<std::string>& values, bool increase);
    void updateBoolSetting();

    void updateTheme(bool increase);
    void updateUSBMode(bool increase);
    void updateLanguage(bool increase);
    void updateOverclock(bool increase);
    void updateShowFPS();
    void restartApplication();
    void quitApplication();
    void updateWifi();
    void updateRotation();
    void updateCoreSelection(bool increase);
    void updateCoreOverride(bool increase);

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

class SystemSettings : public Settings {
public:

    SystemSettings(Configuration& cfg, I18n& i18n, 
                   int minValue, int maxValue, int delta);

    std::vector<Settings::I18nSetting> getSystemSettings();

    /**
     * ISettingsSubject methods 
     */
    std::string getName() override;

};

class FolderSettings : public Settings, public ILanguageObserver {
public:
    FolderSettings(Configuration& cfg, I18n& i18n, 
                   int minValue, int maxValue, int delta);

    std::vector<Settings::I18nSetting> getFolderSettings();

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

        std::map<std::string, ConsoleData> consoleDataMap = cfg.parseIniFile(cfg.get(Configuration::HOME_PATH) + ".simplemenu/section_groups/" + sectionName);

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
        settingsMap[Configuration::CORE_SELECTION] = {Configuration::CORE_SELECTION, currentCore, true};
        notifySettingsChange(Configuration::CORE_SELECTION, currentCore);
    }
};

class RomSettings : public Settings, public ILanguageObserver {
public:
    RomSettings(Configuration& cfg, I18n& i18n, 
                int minValue, int maxValue, int delta);

    std::vector<Settings::I18nSetting> getRomSettings();

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

        std::map<std::string, ConsoleData> consoleDataMap = cfg.parseIniFile(cfg.get(Configuration::HOME_PATH) + ".simplemenu/section_groups/" + sectionName);

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


