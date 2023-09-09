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

struct SaveState {
    std::string menuName;
    int selectedItemPosition;
};

class State {
private:
    Menu* currentMenu;
    RomSettingsMenu* romSettingsMenu;
    SystemSettingsMenu* systemSettingsMenu;
    bool isRomMenuActive = false;
    bool isSystemMenuActive = false;
    SDL_Surface* screen;
    Menu* mainMenu;
    MenuState currentState;
    std::stack<std::pair<Menu*, MenuState>> navigationHistory;

public:
    State(const State&) = delete;
    State& operator=(const State&) = delete;

    static State* getInstance(Menu* initialMenu = nullptr,
                              SystemSettingsMenu* systemSettingsMenu = nullptr, 
                              RomSettingsMenu* romSettingsMenu = nullptr ) {
        if (!instance) {
            instance = new State(initialMenu, systemSettingsMenu, romSettingsMenu);
        }
        return instance;
    }

    State(Menu* initialMenu, SystemSettingsMenu* systemSettingsMenu, RomSettingsMenu* romSettingsMenu) : 
                currentMenu(initialMenu), 
                systemSettingsMenu(systemSettingsMenu), 
                romSettingsMenu(romSettingsMenu), 
                currentState(MenuState::SECTIONS_MENU) {}

    static State* instance;

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

    void saveAppState(const std::string& filePath);
    void loadAppState(const std::string& filePath);

    void showRomSettingsMenuForSelectedROM(MenuItem* selectedROM);
};
