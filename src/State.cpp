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
        if (SubMenuMenuItem* subMenuItem = dynamic_cast<SubMenuMenuItem*>(selectedItem)) {
            subMenuItem->determineAndSetBackground(screen);
        }
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
        if (SubMenuMenuItem* subMenuItem = dynamic_cast<SubMenuMenuItem*>(selectedItem)) {
            subMenuItem->determineAndSetBackground(screen);
        }
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
    int index = currentMenu->getSelectedItemIndex();
    if (SubMenuMenuItem* subMenuItem = dynamic_cast<SubMenuMenuItem*>(currentMenu->getItem(index))) {
        currentMenu = subMenuItem->getSubMenu();
    }

    if (currentMenu == mainMenu) {
        MenuItem* selectedItem = currentMenu->getItem(currentMenu->getSelectedItemIndex());
        if (SubMenuMenuItem* subMenuItem = dynamic_cast<SubMenuMenuItem*>(selectedItem)) {
            subMenuItem->determineAndSetBackground(screen);
        }
    }  
    navigationHistory.push(currentMenu);  // Push the current menu to the stack before changing it.
}

void State::exitFolder() {
    if (!navigationHistory.empty()) {
        currentMenu = navigationHistory.top();  // Set the previous menu as the current one.
        navigationHistory.pop();  // Remove the top menu from the stack.
    }
    
    if (currentMenu->getParent()) {
        currentMenu = currentMenu->getParent();
    }

    if (currentMenu == mainMenu) {
        MenuItem* selectedItem = currentMenu->getItem(currentMenu->getSelectedItemIndex());
        if (SubMenuMenuItem* subMenuItem = dynamic_cast<SubMenuMenuItem*>(selectedItem)) {
            subMenuItem->determineAndSetBackground(screen);
        }
    }  
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
    }
}

void State::showSystemMenu() {
    if (!isRomMenuActive && systemMenu) {
        currentMenu = systemMenu;
    }
}

void State::hideSystemMenu() {
    currentMenu = currentMenu->getParent();
}

bool State::romMenuIsActive() const {
    return isRomMenuActive;
}

void State::hideRomMenu() {
    if (isRomMenuActive) {
        isRomMenuActive = false;
        // Assuming mainMenu is the default menu to return to.
        // Adjust this logic if needed.
        currentMenu = currentMenu->getParent();
    }
}

