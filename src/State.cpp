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
    if (currentIndex- 16 >= 0) {
        currentMenu->setSelectedItemIndex(currentIndex - 16);
        // Reset the selectedRomIndex to the top of the new page
        currentMenu->startIndex = currentIndex - 16;
    }
}

void State::navigateRight() {
    int currentIndex = currentMenu->getSelectedItemIndex();
    if (currentIndex + 16 < currentMenu->getNumberOfItems()) {
        currentMenu->setSelectedItemIndex(currentIndex + 16);
        // Reset the selectedRomIndex to the top of the new page
        currentMenu->startIndex = currentIndex + 16;
    }
}

void State::enterFolder() {
    int index = currentMenu->getSelectedItemIndex();
    if (SubMenuMenuItem* subMenuItem = dynamic_cast<SubMenuMenuItem*>(currentMenu->getItem(index))) {
        currentMenu = subMenuItem->getSubMenu();
    }
}

void State::exitFolder() {
    // Assume each submenu has a pointer to its parent. 
    // This requires updating the Menu class to store a pointer to its parent.
    // If there's no parent (i.e., it's the main menu), this function does nothing.
    if (currentMenu->getParent()) {
        currentMenu = currentMenu->getParent();
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

