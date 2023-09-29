#pragma once

#include <string>
#include <map>
#include <vector>

class Configuration;

class ISettingsObserver {
 public:
  virtual ~ISettingsObserver(){};
  virtual void settingsChanged(const std::string &key, 
                               const std::string &value) = 0;
};

class ISettingsSubject {
 public:
  virtual ~ISettingsSubject(){};
  virtual void attach(ISettingsObserver *observer) = 0;
  virtual void detach(ISettingsObserver *observer) = 0;
  virtual void notifySettingsChange(const std::string &key, const std::string &value) = 0;
};

class Settings {
public: 
    struct Setting {
        std::string key;
        std::string value;
        bool enabled;
    };

private:
    Configuration& cfg;
    std::string currentKey;
    std::string currentValue;

    // Define default settings with their keys
    std::vector<std::string> defaultKeys;

    std::map<std::string, Setting> settingsMap;

    int currentIndex;
    std::vector<std::string> enabledKeys;

public:
    Settings(Configuration& cfg);
    // ~Settings();

    void initializeSettings();

    std::vector<std::string> getEnabledKeys();

    void navigateUp();
    void navigateDown();
    void navigateLeft();
    void navigateRight();
    void navigateEnter();

    void updateVolume(bool increase);
    void updateBrightness(bool increase);
    void updateScreenRefresh(bool increase);

    std::string getCurrentKey() {
        return currentKey;
    };
    std::string getCurrentValue() {
        return settingsMap[currentKey].value;
    };
};
