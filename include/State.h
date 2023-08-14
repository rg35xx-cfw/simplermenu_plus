#pragma once

#include "Menu.h"
#include <stack>

class State {
private:
    Menu* currentMenu;
    RomMenu* romMenu;
    SystemMenu* systemMenu;
    bool isRomMenuActive = false;
    SDL_Surface* screen;
    Menu* mainMenu;
    std::stack<Menu*> navigationHistory;

public:
    State(Menu* initialMenu) : currentMenu(initialMenu), romMenu(new RomMenu()), systemMenu(new SystemMenu()) {}

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
    bool romMenuIsActive() const ;
    void hideRomMenu();
    void showSystemMenu();
    void hideSystemMenu();
};
