#pragma once

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <memory>
#include <string>
#include <SDL/SDL.h>

#include "Configuration.h"
#include "Menu.h"
#include "MenuItem.h"
#include "ThumbnailCache.h"
#include "State.h"
#include "FileManager.h"

namespace pt = boost::property_tree;

class State;
class SystemMenu;
class RomMenu;
class SystemMenu;
class SimpleMenuItem;

enum class SettingId {
    VOLUME,
    BRIGHTNESS,
    SCREEN_REFRESH,
    SHOW_FPS,
    OVERCLOCK,
    THEME,
    USB_MODE,
    WIFI,
    QUIT

};

class Application : public ISettingsObserver {
private:

    // TODO should this be constant?
    std::map<SettingId, std::string> idToString{
        {SettingId::VOLUME, "System.volume"}, 
        {SettingId::BRIGHTNESS, "System.brightness"}, 
        {SettingId::SCREEN_REFRESH, "System.screenRefresh"},
        {SettingId::SHOW_FPS, "System.showFPS"},
        {SettingId::OVERCLOCK, "System.overclock"},
        {SettingId::THEME, "System.theme"},
        {SettingId::USB_MODE, "System.usbMode"},
        {SettingId::WIFI, "System.wifi"},
        {SettingId::QUIT, "System.quit"}
    };

    // const std::string VOLUME_ID = "System.volume";
    // const std::string BRIGHTNESS_ID = "System.brightness";
    // const std::string SCREEN_REFRESH_ID = "System.screenRefresh";
    // const std::string SHOW_FPS_ID = "System.showFPS";
    // const std::string OVERCLOCK_ID = "System.overclock";
    // const std::string THEME_ID = "System.theme";


    std::unique_ptr<Menu> mainMenu;
    std::unique_ptr<State> currentState;
    std::unique_ptr<SystemMenu> systemMenu;
    std::unique_ptr<RomMenu> romMenu;
    ThumbnailCache thumbnailCache;
    SDL_Surface* screen;
    SDL_Surface* background = nullptr;
    static Application* instance;
    SDL_Surface* folderBackgroundSurface = nullptr;
    std::string mainFont;
    int mainFontSize;
    int screenWidth;
    int screenHeight;
    int screenDepth;

    Configuration& cfg = Configuration::getInstance();

    // std::map<std::string, int> intSettings;

    SDL_Joystick *joystick = nullptr;

public:
    Application();

    ~Application() {
        TTF_CloseFont(font);
        TTF_Quit();
        if (joystick) {
            SDL_JoystickClose(joystick);
            joystick = nullptr;
        }
        SDL_Quit();
    }

    TTF_Font* font;

    bool fileExists(const std::string& filename);

    ThumbnailCache& getThumbnailCache();

    static Application& getInstance();

    void run();

    State* getCurrentState() const;

    // Menu* getMainMenu() const;

    void showMainMenu();

    void setBackgroundForFolder(const std::string& folderName, SDL_Surface* screen);

    SDL_Surface* getBackground() const;

    void clearBackground();


    /**
     * ISettingsObserver methods
    */
    void settingsChanged(const std::string &id, 
                         const std::string &value) override;

private:
    void setupMenu();

    void createSystemMenu();

    void printFPS(int fps);

    void handleKeyPress(SDLKey key);

    void handleJoystickEvents(SDL_Event& event);

    void loadMenuFromJSON(const std::string& jsonPath);


};
