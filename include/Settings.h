#pragma once

#include <string>

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
