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

enum class SettingsType {
    SYSTEM,
    ROM
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
    Configuration& cfg;
    I18n& i18n;
    std::string currentKey;
    std::string currentValue;

    // Define default settings with their keys
    std::vector<std::string> defaultKeys;

    std::map<std::string, Setting> settingsMap;

    int currentIndex;
    std::vector<std::string> enabledKeys;

    std::vector<ISettingsObserver *> observers;

    std::set<std::string> themeFolders;

public:
    Settings(Configuration& cfg, I18n& i18n, ISettingsObserver *observer, SettingsType type);
    // ~Settings();

    void initializeSettings();

    std::vector<std::string> getEnabledKeys();

    void navigateUp();
    void navigateDown();
    void navigateLeft();
    void navigateRight();
    void navigateEnter();

    std::vector<I18nSetting> getSystemSettings();
    std::vector<I18nSetting> getRomSettings();

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
