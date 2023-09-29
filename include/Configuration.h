#pragma once
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include <map>
#include <set>
#include <vector>
#include <memory>
#include "Settings.h"

struct ConsoleData {
    std::string name;
    std::vector<std::string> execs;
    std::vector<std::string> romExts;
    std::vector<std::string> romDirs;
};

struct SettingsMenuItem {
    std::string id;
    std::string type;
    std::string title;
    std::vector<std::string> options; // Empty for menu items that don't have options

    // Constructor
    SettingsMenuItem(const std::string& id, const std::string& type, 
             const std::string& title, const std::vector<std::string>& options = {})
        : id(id), type(type), title(title), options(options) {}
};

class Configuration : public ISettingsSubject {
private:

    std::string configIniFilepath;

    boost::property_tree::ptree mainPt;

    std::vector<ISettingsObserver*> observers;

public:

    Configuration(const std::string& configIniFilepath);

    void set(const std::string& id, const std::string& value);

    std::string get(const std::string& id) const;
    bool getBool(const std::string& id) const;
    int getInt(const std::string& id) const;

    std::set<std::string> getList(const std::string& id, 
                                  const char delimiter = ',') const;
    std::string getThemePath() const;
    std::map<std::string, ConsoleData> parseIniFile(const std::string& iniPath);

    void saveConfigIni();

    int getSectionSize(std::string section) {
        return mainPt.get_child(section).size();
    }

    boost::property_tree::ptree getSection(const std::string& section) {
        return mainPt.get_child(section);
    }

    std::string getKeyByIndex(const std::string& sectionName, std::size_t index) {
        auto section = mainPt.get_child(sectionName);
        std::size_t currentIndex = 0;
        for (const auto& kv : section) {
            if (currentIndex == index) {
                return kv.first;
            }
            ++currentIndex;
        }
        throw std::out_of_range("Index out of range");
    }

    std::string getValueByIndex(const std::string& sectionName, std::size_t index) {
        auto section = mainPt.get_child(sectionName);
        std::size_t currentIndex = 0;
        for (const auto& kv : section) {
            if (currentIndex == index) {
                return kv.second.get_value<std::string>();
            }
            ++currentIndex;
        }
        throw std::out_of_range("Index out of range");
    }

        void attach(ISettingsObserver *observer) override {
        observers.push_back(observer);
    }

    void detach(ISettingsObserver *observer) override {
        observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
    }

    // void notifySettingsChange() override {
    //     for (ISettingsObserver *observer : observers) {
    //         // For simplicity, we're notifying all observers about all changes.
    //         // You can make this more fine-grained by specifying the key and value that changed.
    //         observer->settingsChanged("", ""); 
    //     }
    // }

    void notifySettingsChange(const std::string &key, const std::string &value) override{
        for (ISettingsObserver *observer : observers) {
            observer->settingsChanged(key, value); 
        }
    }


};
