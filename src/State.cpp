#include "State.h"
#include "Menu.h"

void State::navigateUp() {
    int prevIndex = currentMenu->getSelectedItemIndex();
    currentMenu->navigateUp();
    int newIndex = currentMenu->getSelectedItemIndex();

    // Deselect the previous item and select the new one
    if (auto item = dynamic_cast<SimpleMenuItem*>(currentMenu->getItem(prevIndex))) {
        item->deselect();
    }
    if (auto item = dynamic_cast<SimpleMenuItem*>(currentMenu->getItem(newIndex))) {
        item->select();
    }
}

void State::navigateDown() {
    int prevIndex = currentMenu->getSelectedItemIndex();
    currentMenu->navigateDown();
    int newIndex = currentMenu->getSelectedItemIndex();

    // Deselect the previous item and select the new one
    if (auto item = dynamic_cast<SimpleMenuItem*>(currentMenu->getItem(prevIndex))) {
        item->deselect();
    }
    if (auto item = dynamic_cast<SimpleMenuItem*>(currentMenu->getItem(newIndex))) {
        item->select();
    }
}

void State::navigateLeft() {
    int currentIndex = currentMenu->getSelectedItemIndex();
        std::cout << "Navigating Left from index: " << currentIndex << std::endl;

    if (currentIndex - 16 >= 0) {
        currentMenu->setSelectedItemIndex(currentIndex - 16);
        // Reset the selectedRomIndex to the top of the new page
        currentMenu->startIndex = currentIndex - 16;
    }
    currentMenu->navigateLeft();

    MenuItem* selectedItem = currentMenu->getItem(currentIndex);
    if (selectedItem) {
        selectedItem->navigateLeft();
    }
}

void State::navigateRight() {
    int currentIndex = currentMenu->getSelectedItemIndex();
        std::cout << "Navigating Right from index: " << currentIndex << std::endl;

    if (currentIndex + 16 < currentMenu->getNumberOfItems()) {
        currentMenu->setSelectedItemIndex(currentIndex + 16);
        // Reset the selectedRomIndex to the top of the new page
        currentMenu->startIndex = currentIndex + 16;
    }
    currentMenu->navigateRight();

    MenuItem* selectedItem = currentMenu->getItem(currentIndex);
    if (selectedItem) {
        selectedItem->navigateRight();
    }
}

void State::enterFolder() {
    std::cout << "enterFolder, IN state: " << getActiveMenuName() << std::endl;
    navigationHistory.push({currentMenu, currentState});

    switch (currentState) {
        case MenuState::SECTIONS_MENU:
        {
            int index = currentMenu->getSelectedItemIndex();
            if (MenuItem* menuItem = currentMenu->getItem(index)) {
                currentMenu = menuItem->getSubMenu();
                currentState = MenuState::SYSTEMS_MENU;
            }
            break;
        }
        case MenuState::SYSTEMS_MENU:
            // Logic to transition from SYSTEMS_MENU to ROMLIST_MENU
            {
            int index = currentMenu->getSelectedItemIndex();
            if (MenuItem* menuItem = currentMenu->getItem(index)) {
                currentMenu = menuItem->getSubMenu();
                currentState = MenuState::ROMLIST_MENU;
            }
            // Handle the transition, e.g., load ROMs for the selected system
            break;
            }

        case MenuState::ROMLIST_MENU:
            currentMenu->getItem(currentMenu->getSelectedItemIndex())->executeAction();

            // Logic to interact with a specific ROM or enter its settings
            // For example, if the selected ROM has additional settings or options, you'd transition to ROM_SETTINGS_MENU
            break;

        case MenuState::SYSTEM_SETTINGS_MENU:
            currentMenu->getItem(currentMenu->getSelectedItemIndex())->executeAction();

            // Logic to handle entering a specific system setting
            break;

        case MenuState::ROM_SETTINGS_MENU:
            currentMenu->getItem(currentMenu->getSelectedItemIndex())->executeAction();

            // Logic to handle entering a specific ROM setting
            break;

        default:
            break;
    }

    std::cout << "enterFolder, OUT state: " << getActiveMenuName() << std::endl;
}

void State::exitFolder() {
    std::cout << "exitFolder, IN state: " << getActiveMenuName() << std::endl;
    
    if (!navigationHistory.empty()) {
        auto [prevMenu, prevState] = navigationHistory.top();
        currentMenu = prevMenu;
        currentState = prevState;
        navigationHistory.pop();
    } else {
        std::cerr << "Error: Navigation history is empty!" << std::endl;
        return; // or handle the error differently
    }

    std::cout << "exitFolder, OUT state: " << getActiveMenuName() << std::endl;
}

void State::printCurrentContents() {
    currentMenu->printContents();
}

Menu* State::getCurrentMenu() {
    return currentMenu;
}

void State::setCurrentMenu(Menu* newMenu) {
    currentMenu = newMenu;
}

void State::showRomMenu() {
    navigationHistory.push({currentMenu, currentState});
    // Only enter the RomSettings Menu from the romlist
    if (currentState == MenuState::ROMLIST_MENU) {
        // if (!isRomMenuActive && romMenu) {
        if (romMenu) {
            isRomMenuActive = true;
            currentMenu = romMenu;
            currentState = MenuState::ROM_SETTINGS_MENU;
        }
    } else {
        std::cout << "Not in romlist menu, ignoring" << std::endl;
    }
}

void State::showSystemMenu() {
    navigationHistory.push({currentMenu, currentState});
    if (systemMenu) {
        isSystemMenuActive = true;
        currentMenu = systemMenu;
        currentState = MenuState::SYSTEM_SETTINGS_MENU;
    }
}

std::string State::getActiveMenuName() const {
    switch (currentState) {
        case MenuState::SECTIONS_MENU:
            return "SectionsMenu";
        case MenuState::ROMLIST_MENU:
            return "RomListMenu";
        case MenuState::ROM_SETTINGS_MENU:
            return "RomSettingsMenu";
        case MenuState::SYSTEM_SETTINGS_MENU:
            return "SystemSettingsMenu";
        case MenuState::SYSTEMS_MENU:
        default:
            return "MainMenu";
    }
}
