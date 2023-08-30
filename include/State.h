#pragma once

#include <stack>
#include "Menu.h"

enum class MenuState {
    SECTIONS_MENU,
    SYSTEMS_MENU,
    ROMLIST_MENU,
    SYSTEM_SETTINGS_MENU,
    ROM_SETTINGS_MENU
};

class State {
private:
    Menu* currentMenu;
    RomMenu* romMenu;
    SystemMenu* systemMenu;
    bool isRomMenuActive = false;
    bool isSystemMenuActive = false;
    SDL_Surface* screen;
    Menu* mainMenu;
    MenuState currentState;
    std::stack<std::pair<Menu*, MenuState>> navigationHistory;


public:
    State(Menu* initialMenu, SystemMenu* systemMenu) : currentMenu(initialMenu), romMenu(new RomMenu()), systemMenu(systemMenu), currentState(MenuState::SECTIONS_MENU) {}

    void navigateUp();
    void navigateDown();
    void navigateLeft();
    void navigateRight();
    void enterFolder();
    void exitFolder();
    void printCurrentContents();
    Menu* getCurrentMenu();
    void setCurrentMenu(Menu* newMenu);
    void showRomMenu() ;
    void showSystemMenu();

    MenuState getCurrentState() const {
        return currentState;
    }

    std::string getActiveMenuName() const;
};
