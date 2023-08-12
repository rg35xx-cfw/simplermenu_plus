#pragma once

#include "Menu.h"

class State {
private:
    Menu* currentMenu;
    RomMenu* romMenu;
    bool isRomMenuActive = false;


public:
    State(Menu* initialMenu) : currentMenu(initialMenu), romMenu(new RomMenu()) {}

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
};
