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
#include "ControlMapping.h"

namespace pt = boost::property_tree;

class State;
class SystemMenu;
class RomMenu;
class SystemMenu;
class SimpleMenuItem;


class Application : public ISettingsObserver {
private:
    std::unique_ptr<Menu> mainMenu;
    State* currentState;
    std::unique_ptr<SystemSettingsMenu> systemSettingsMenu;
    std::unique_ptr<RomSettingsMenu> romSettingsMenu;
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
    Theme& theme = Theme::getInstance();

    SDL_Joystick *joystick = nullptr;

    ControlMapping& controlMapping;

    int buttonPressed = -1;
    Uint32 repeatStartTime = 0;
    Uint32 lastButtonPressTime = 0;
    Uint32 nextRepeatTime = 0;
    const Uint32 initialDelay = 500;  // for example, 500ms
    const Uint32 repeatInterval = 50;  // for example, 100ms

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

    static Application* getInstance() {
        if (!instance) {
            instance = new Application();
        }
        return instance;
    }

    TTF_Font* font;

    bool fileExists(const std::string& filename);

    ThumbnailCache& getThumbnailCache();

    SDL_Surface* getScreen() {
        return screen;
    }

    void run();

    State* getCurrentState() const;

    void showMainMenu();

    void setBackgroundForFolder(const std::string& folderName, SDL_Surface* screen);

    SDL_Surface* getBackground() const;

    void clearBackground();


    /**
     * ISettingsObserver methods
    */
    void settingsChanged(const SettingId &id, 
                         const std::string &value) override;

private:
    void setupMenu();

    void createSystemSettingsMenu();

    void createRomSettingsMenu();

    void printFPS(int fps);

    void handleKeyPress(SDLKey key);

    void handleJoystickEvents(SDL_Event& event);

    void loadMenuFromJSON(const std::string& jsonPath);


};
