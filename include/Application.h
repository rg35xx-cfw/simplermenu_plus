#pragma once

#include <memory>
#include <string>
#include <SDL/SDL.h>
#include "Configuration.h"
#include "Menu.h"
#include "MenuItem.h"
#include "ThumbnailCache.h"
#include "State.h"
#include "FileManager.h"

class State;
class SystemMenu;
class RomMenu;
class SystemMenu;
class SimpleMenuItem;

class Application {
private:
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

private:
    void setupMenu();

    void handleKeyPress(SDLKey key);

    void handleJoystickEvents(SDL_Event& event);

};
