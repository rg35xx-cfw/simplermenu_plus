#include "I18n.h"
#include "Exception.h"
#include <iostream>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string.hpp>


/////////
// CONFIG.INI
////////

const std::string I18n::SYSTEM_SETTINGS = std::string("systemSettings");
const std::string I18n::ROM_SETTINGS = std::string("romSettings");


I18n::I18n(const std::string& i18nFilepath) 
    : i18nFilepath(i18nFilepath) {

    // Load values from .ini file using Boost.PropertyTree
    boost::property_tree::ini_parser::read_ini(i18nFilepath, mainPt);

    // Get languages list
    for (const auto& section : this->mainPt) {
        std::string normalizedLang = boost::algorithm::to_upper_copy(section.first);
        languages.insert(normalizedLang);
    }

    // Set default language
    lang = *(languages.begin());
}

std::string I18n::getLang() const {
    return lang;
}

void I18n::setLang(const std::string& newLang) {
    std::string normalizedLang = boost::algorithm::to_upper_copy(newLang);
    if (languages.find(normalizedLang) != languages.end()) {
        lang = normalizedLang;
    } else {
        throw ItemNotFoundException("Language unknown: " + newLang);
    }
    
}

std::string I18n::get(const std::string& id) const {
    // TODO exceptions  ptree_bad_path, ptree_bad_data, both from ptree_error 
    return mainPt.get<std::string>(lang + "." + id);
}

std::set<std::string> I18n::getLanguages() const {
    return languages;
}
