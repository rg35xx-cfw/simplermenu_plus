#include "Menu.h"

Menu::Menu() {
    itemsPerPage = Configuration::getInstance().getIntValue("Menu.itemsPerPage");
    listOffset_x = Configuration::getInstance().getIntValue("Menu.listOffset_x");
    listOffset_y = Configuration::getInstance().getIntValue("Menu.listOffset_y"); 
}

void Menu::print() const {
    for (const auto& item : items) {
        std::cout << item->getName() << std::endl;
    }
}

void Menu::navigateUp() {
    if (selectedItemIndex > 0) {
        selectedItemIndex--;

        // If the selected item is above the current view, adjust the start index
        if (selectedItemIndex < startIndex) {
            startIndex = selectedItemIndex;
        }    
    }

    // Adjust the currentPage if necessary
    handlePages();
}

void Menu::navigateDown() {
    if (selectedItemIndex < static_cast<int>(items.size()) - 1) {
        selectedItemIndex++;

        // If the selected item is below the current view, adjust the start index
        if (selectedItemIndex >= startIndex + itemsPerPage) {
            startIndex = selectedItemIndex - itemsPerPage + 1;
        }
    }

    // Adjust the currentPage if necessary
    handlePages();    
}

void Menu::selectItem() {
    if (selectedItemIndex >= 0 && selectedItemIndex < static_cast<int>(items.size())) {
        items[selectedItemIndex]->executeAction();
    }
}

void Menu::addItem(std::unique_ptr<MenuItem> item) {
    items.push_back(std::move(item));
}

int Menu::getNumberOfItems() {
    return static_cast<int>(items.size());
}

void Menu::render(SDL_Surface* screen, TTF_Font* font) {
    int x = listOffset_x;  // Starting x-coordinate for rendering
    int y = listOffset_y;  // Starting y-coordinate for rendering

    int endIndex = std::min(startIndex + itemsPerPage, static_cast<int>(items.size()));

    total_pages = (items.size() + itemsPerPage -1) / itemsPerPage;

    for (int i = startIndex; i < endIndex; i++) {
        bool isSelected = (i == selectedItemIndex);
        items[i]->render(screen, font, x, y + (i - startIndex) * 24, isSelected);  // Adjusting y for each item
    }

    // Display pagination page number / total_pages at the bottom
    std::string pageInfo = std::to_string(currentPage) + " / " + std::to_string(total_pages);
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, pageInfo.c_str(), {255,255,255});
    SDL_Rect destRect = {100, 445, 0, 0};  // Adjust x position by scrollPixelPosition
    SDL_BlitSurface(textSurface, NULL, screen, &destRect);

}

int Menu::getSelectedItemIndex() const {
    return selectedItemIndex;
}

void Menu::setSelectedItemIndex(int index) {
    selectedItemIndex = index;
}

MenuItem* Menu::getItem(int index) {
    if (index >= 0 && index < static_cast<int>(items.size())) {
        return items[index].get();
    }
    return nullptr; // or handle the error differently
}

void Menu::setParent(Menu* parentMenu) {
    parent = parentMenu;
}

Menu* Menu::getParent() const {
    return parent;
}

bool Menu::isRomMenu() const {
    return false;
}

void Menu::handlePages() {
    currentPage = selectedItemIndex / itemsPerPage;
    startIndex = currentPage * itemsPerPage;
}

void Menu::printContents(int indentLevel) const {
    std::string indent(indentLevel * 2, ' ');  // Use two spaces per indent level
    for (const auto& item : items) {
        std::cout << indent << item->getName() << std::endl;
        
        // If the item is a submenu, print its contents recursively
        if (auto subMenuItem = dynamic_cast<SubMenuMenuItem*>(item.get())) {
            subMenuItem->getSubMenu()->printContents(indentLevel + 1);
        }
    }
}

SystemMenu::SystemMenu() {}

RomMenu::RomMenu() {
    // addItem(std::make_unique<SimpleMenuItem>("Start Game"));
    addItem(std::make_unique<SimpleMenuItem>("Game Settings"));
    addItem(std::make_unique<SimpleMenuItem>("Auto Load State"));
    addItem(std::make_unique<SimpleMenuItem>("Auto Save State"));
    addItem(std::make_unique<SimpleMenuItem>("Load State"));
    addItem(std::make_unique<SimpleMenuItem>("Overclock"));
    addItem(std::make_unique<SimpleMenuItem>("Return to Main Menu"));
}

bool RomMenu::isRomMenu() const {
    return true;
}

