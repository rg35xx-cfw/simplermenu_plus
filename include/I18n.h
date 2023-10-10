#pragma once
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include <map>
#include <set>
#include <vector>
#include <memory>


class I18n {
private:

    std::string i18nFilepath;

    boost::property_tree::ptree mainPt;

    std::set<std::string> languages;

    std::string lang;

public:

    /////////
    // I18N.INI
    ////////

    static const std::string SYSTEM_SETTINGS;
    
    static const std::string ROM_SETTINGS;

    I18n(const std::string& i18nFilepath);

    std::string getLang() const;
    void setLang(const std::string& newLang);

    std::string get(const std::string& id) const;
    
    std::set<std::string> getLanguages() const;
};
