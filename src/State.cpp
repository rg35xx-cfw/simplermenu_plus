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

    if (currentMenu == mainMenu) {
        MenuItem* selectedItem = currentMenu->getItem(currentMenu->getSelectedItemIndex());
        selectedItem->determineAndSetBackground(screen);
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

    if (currentMenu == mainMenu) {
        MenuItem* selectedItem = currentMenu->getItem(currentMenu->getSelectedItemIndex());
        selectedItem->determineAndSetBackground(screen);
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

// void State::enterFolder() {
//     int index = currentMenu->getSelectedItemIndex();
//     if (MenuItem* menuItem = currentMenu->getItem(index)) {
//         currentMenu = menuItem->getSubMenu();
//         currentState = MenuState::ROMLIST_MENU;
//     }

//     if (currentMenu != mainMenu) {
//         MenuItem* selectedItem = currentMenu->getItem(currentMenu->getSelectedItemIndex());
//         if (MenuItem* menuItem = selectedItem) {
//             menuItem->determineAndSetBackground(screen);
//         }
//     } 

//     if (currentMenu == systemMenu || currentMenu == romMenu || isRomMenuActive) {
//         currentMenu->getItem(currentMenu->getSelectedItemIndex())->executeAction();
//     } 
//     navigationHistory.push(currentMenu);  // Push the current menu to the stack before changing it.
// }

void State::enterFolder() {
    std::cout << "enterFolder, IN state: " << getActiveMenuName() << std::endl;

    switch (currentState) {
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

    // Any common logic for entering a folder, regardless of the current state
    navigationHistory.push(currentMenu);  // Push the current menu to the stack before changing it.
}


void State::exitFolder() {
    std::cout << "exitFolder, IN state: " << getActiveMenuName() << std::endl;
    switch (currentState) {
        // case MenuState::SYSTEMS_MENU:
        //     // Logic to transition from SYSTEMS_MENU to ROMLIST_MENU
        //     {
        //     int index = currentMenu->getSelectedItemIndex();
        //     if (MenuItem* menuItem = currentMenu->getItem(index)) {
        //         currentMenu = menuItem->getSubMenu();
        //         currentState = MenuState::ROMLIST_MENU;
        //     }
        //     // Handle the transition, e.g., load ROMs for the selected system
        //     break;
        //     }

        case MenuState::ROMLIST_MENU:
            currentMenu = currentMenu->getParent();
            currentState = MenuState::SYSTEMS_MENU;
            break;
        case MenuState::SYSTEM_SETTINGS_MENU:
            std::cout << "navigating back " << std::endl;
            currentMenu = navigationHistory.top();  // Set the previous menu as the current one.
            navigationHistory.pop();  // Remove the top menu from the stack.
            currentState = MenuState::SYSTEMS_MENU;
            break;
        case MenuState::ROM_SETTINGS_MENU:
        {
            //currentMenu = currentMenu->getParent();
            currentMenu = navigationHistory.top(); 
            navigationHistory.pop();
            // MenuItem* selectedItem = currentMenu->getItem(currentMenu->getSelectedItemIndex());
            // selectedItem->determineAndSetBackground(screen);
            currentState = MenuState::ROMLIST_MENU;
            break;
        }
        default:
            break;
    }
    // if (!navigationHistory.empty()) {
    //     std::cout << "navigating back " << std::endl;
    //     currentMenu = navigationHistory.top();  // Set the previous menu as the current one.
    //     navigationHistory.pop();  // Remove the top menu from the stack.
    // }
    
    // if (currentMenu->getParent()) {
    //     currentMenu = currentMenu->getParent();
    //     currentState = MenuState::SYSTEMS_MENU;
    // }

    // if (currentMenu == mainMenu) {
    //     MenuItem* selectedItem = currentMenu->getItem(currentMenu->getSelectedItemIndex());
    //     selectedItem->determineAndSetBackground(screen);
    // }  
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
    if (!isRomMenuActive && romMenu) {
        isRomMenuActive = true;
        currentMenu = romMenu;
        currentState = MenuState::ROM_SETTINGS_MENU;
    }
}

void State::showSystemMenu() {
    if (!isRomMenuActive && systemMenu) {
        isSystemMenuActive = true;
        currentMenu = systemMenu;
        currentState = MenuState::SYSTEM_SETTINGS_MENU;
    }
}

void State::hideSystemMenu() {
    currentMenu = currentMenu->getParent();
    currentState = MenuState::SYSTEMS_MENU;
}

bool State::romMenuIsActive() const {
    return isRomMenuActive;
}

bool State::systemMenuIsActive() const {
    return isSystemMenuActive;
}

void State::hideRomMenu() {
    if (isRomMenuActive) {
        isRomMenuActive = false;
        currentMenu = currentMenu->getParent();
        currentState = MenuState::SYSTEMS_MENU;
    }
}

std::string State::getActiveMenuName() const {
    switch (currentState) {
        case MenuState::ROMLIST_MENU:
            return "RomListMenu";
        case MenuState::ROM_SETTINGS_MENU:
            return "RomMenu";
        case MenuState::SYSTEM_SETTINGS_MENU:
            return "SystemMenu";
        case MenuState::SYSTEMS_MENU:
        default:
            return "MainMenu";
    }
}
