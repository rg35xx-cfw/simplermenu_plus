#pragma once

#include <string>
#include <map>
#include <vector>
#include "I18n.h"

class Configuration;

class ISettingsObserver {
 public:
  virtual ~ISettingsObserver(){};
  virtual void settingsChanged(const std::string &key, 
                               const std::string &value) = 0;
  virtual std::string getName() = 0;
};

class ISettingsSubject {
 public:
  virtual ~ISettingsSubject(){};
  virtual void attach(ISettingsObserver *observer) = 0;
  virtual void detach(ISettingsObserver *observer) = 0;
  virtual void notifySettingsChange(const std::string &key, const std::string &value) = 0;
  virtual std::string getName() = 0;
};

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
     std::set<std::string> cores;

protected:
    Configuration& cfg;
    I18n& i18n;

    std::vector<std::string> enabledKeys;

    std::map<std::string, Setting> settingsMap;

    std::vector<std::string> getEnabledKeys();

public:
    Settings(Configuration& cfg, I18n& i18n, ISettingsObserver *observer);
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

    void updateVolume(bool increase);
    void updateBrightness(bool increase);
    void updateScreenRefresh(bool increase);
    void updateTheme(bool increase);
    void updateUSBMode(bool increase);
    void updateLanguage(bool increase);
    void updateOverclock(bool increase);
    void updateShowFPS();
    void saveSettings();
    void restartApplication();
    void quitApplication();
    void updateWifi();
    void updateRotation();
    void updateCoreOverride(bool increase);

    /**
     * ISettingsSubject methods
    */
    void attach(ISettingsObserver *observer) override;
    void detach(ISettingsObserver *observer) override;
    void notifySettingsChange(const std::string &key, const std::string &value) override;
    std::string getName() override;
    

    std::string getCurrentKey() {
        return currentKey;
    };
    std::string getCurrentValue() {
        return settingsMap[currentKey].value;
    };
};

class RomSettings : public Settings {
public:
    RomSettings(Configuration& cfg, I18n& i18n, ISettingsObserver *observer)
        : Settings(cfg, i18n, observer) {
        defaultKeys = {
            Configuration::ROM_OVERCLOCK, Configuration::ROM_AUTOSTART, Configuration::CORE_OVERRIDE
        };    

        attach(observer);

        initializeSettings();

        enabledKeys = getEnabledKeys();
    }

    std::vector<Settings::I18nSetting> getRomSettings();

};

class SystemSettings : public Settings {
public:
    SystemSettings(Configuration& cfg, I18n& i18n, ISettingsObserver *observer)
        : Settings(cfg, i18n, observer) {
        defaultKeys = {
            Configuration::VOLUME, Configuration::BRIGHTNESS, Configuration::SCREEN_REFRESH,
            Configuration::SHOW_FPS, Configuration::OVERCLOCK, Configuration::THEME,
            Configuration::USB_MODE, Configuration::WIFI, Configuration::ROTATION,
            Configuration::LANGUAGE,
            Configuration::UPDATE_CACHES, Configuration::SAVE_SETTINGS, Configuration::RESTART, 
            Configuration::QUIT
        };

        attach(observer);

        initializeSettings();



        enabledKeys = getEnabledKeys();
    }

    std::vector<Settings::I18nSetting> getSystemSettings();
};

