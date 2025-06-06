#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <set>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include "RenderComponent.h"
#include "Configuration.h"
#include "Theme.h"
#include "FileManager.h"
#include "ControlMapping.h"
#include "MenuCache.h"
#include "Menu.h"
#include "HelperUtils.h"
#include "Settings.h"
#include "I18n.h"

namespace pt = boost::property_tree;

class Configuration;
class RenderComponent;


class Application : public ISettingsObserver, public ILanguageSubject {
private:
    Menu menu;
    MenuCache menuCache;

    TTF_Font* font;
    SDL_Joystick *joystick = nullptr;

    Configuration cfg;
    Theme theme;
    I18n i18n;
    
    AppSettings appSettings;
    SystemSettings systemSettings;
    RomSettings romSettings;

    ControlMapping controlMapping;
    RenderComponent renderComponent;

    HelperUtils helper;

    std::vector<ILanguageObserver *> langObservers;

    // MenuLevel currentMenuLevel = MENU_SECTION;

    State state;

    // int currentSectionIndex = 0;
    // int currentFolderIndex = 0;
    // int currentRomIndex = 0;
    int currentSettingsIndex = 0;
    int currentFolderSettingsIndex = 0;
    int currentRomSettingsIndex = 0;
    int currentSettingsValue = 0;

    bool isButtonHeld;
    SDL_Event lastHeldEvent;
    unsigned int repeatStartTime;
    unsigned int repeatInterval;

    void loadCache(bool force = false);

    std::vector<CachedMenuItem> populateCache();

    void populateMenu(Menu& menu);

public:
    Application();
    // ~Application();

    void drawCurrentState();

    void handleCommand(ControlMap cmd);

    void run();

    void print_list();
    
    void launchRom();

    bool isInteger(const std::string &s);

    /**
     * ISettingsObserver methods
    */
    void settingsChanged(const std::string &key, const std::string &value) override;

    /**
     * ILanguageSubject methods
     */
    void attach(ILanguageObserver *observer) override;
    void detach(ILanguageObserver *observer) override;
    void notifyLanguageChange() override;

    /**
     * ISettingsObserver and ILanguageSubject common methods 
     */
    std::string getName() override;

};
