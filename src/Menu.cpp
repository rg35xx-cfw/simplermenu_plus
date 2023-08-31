#include "Menu.h"
#include "State.h"

Menu::Menu() {

    this->itemsPerPage = this->cfg.getIntValue(SettingId::ITEMS_PER_PAGE);

    this->customSpacing = 0;
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

void Menu::navigateLeft() {
    std::cout << "menu::navigateLeft" << std::endl;
}

void Menu::navigateRight() {
    std::cout << "menu::navigateRight" << std::endl;
}

void Menu::selectItem() {
    if (selectedItemIndex >= 0 && selectedItemIndex < static_cast<int>(items.size())) {
        items[selectedItemIndex]->executeAction();
    }
}

void Menu::addItem(std::unique_ptr<SimpleMenuItem> item) {
    item->setParentMenu(this); // Set the parent menu
    items.push_back(std::move(item));
}

int Menu::getNumberOfItems() {
    return static_cast<int>(items.size());
}

void Menu::render(SDL_Surface* screen, TTF_Font* font, MenuState currentState) {
   // Check if the menu is a ROM menu and set the background
    if (currentState == MenuState::ROMLIST_MENU) {
        SDL_Surface* romBackground = SimpleMenuItem::loadRomBackground();
        if (romBackground) {
            SDL_BlitSurface(romBackground, NULL, screen, NULL);
            SDL_FreeSurface(romBackground);  // Free the surface after using it
        }
    } else {
        // Existing logic to set the background for other views
        MenuItem* selectedItem = items[selectedItemIndex].get();
        SDL_Surface* background = selectedItem->getAssociatedBackground();
        if (background) {
            SDL_BlitSurface(background, NULL, screen, NULL);
        } else {
            SDL_FillRect(screen, nullptr, SDL_MapRGB(screen->format, 0, 0, 0));
        }
    }

    // 3. Render each menu item
    int x = customFontPath.empty() ? 
        this->cfg.getIntValue(SettingId::LIST_OFFSET_X) : itemOffset_x;
    int y = customFontPath.empty() ? 
        this->cfg.getIntValue(SettingId::LIST_OFFSET_Y) : itemOffset_y;

    // Open customFont if defined (used for settings menu)
    TTF_Font* currentFont = customFontPath.empty() ? font : TTF_OpenFont(customFontPath.c_str(), customFontSize);

    int spacing = customSpacing ? customSpacing : 24;

    if (useSelectionRectangle || (currentState == MenuState::ROMLIST_MENU)) {
        SDL_Rect rectangle;
        if (currentState == MenuState::ROMLIST_MENU) {
            rectangle.x = x - 1;
            rectangle.y = y + (selectedItemIndex - startIndex) * spacing; // Adjust for current item position
        } else {
            rectangle.x = x - 10;
            rectangle.y = y - 5 + (selectedItemIndex - startIndex) * spacing; // Adjust for current item position
        }
        rectangle.w = (selectionRectangleWidth > 0) ? selectionRectangleWidth : 225; // screen width as fallback
        rectangle.h = selectionRectangleHeight;

        SDL_FillRect(screen, &rectangle, SDL_MapRGB(screen->format, selectionRectangleColor.r, selectionRectangleColor.g, selectionRectangleColor.b));
    }

    int endIndex = std::min(startIndex + itemsPerPage, static_cast<int>(items.size()));
    for (int i = startIndex; i < endIndex; i++) {
        bool isSelected = (i == selectedItemIndex);
        int spacing = customSpacing ? customSpacing : 24;
        items[i]->render(screen, currentFont, x, y + (i - startIndex) * spacing, isSelected, currentState);
    }

    // If we are in the rom list view, we render the number of pages as well as the system title
    if (currentState == MenuState::ROMLIST_MENU) {

        MenuItem* selectedItem = items[selectedItemIndex].get();

        std::string systemTitle = selectedItem->getFolderName() + "TEST";
        transform(systemTitle.begin(), systemTitle.end(), systemTitle.begin(), ::toupper);
        std::string fontPath = this->cfg.getValue(SettingId::TITLE_FONT);
        TTF_Font* titleFont = TTF_OpenFont(fontPath.c_str(), 32);

        SDL_Surface* titleSurface = TTF_RenderText_Blended(titleFont, systemTitle.c_str(), {255,255,255});
        SDL_Rect destRect = {320 - titleSurface->w/2, 12, 0, 0};  // Position for System title
        SDL_BlitSurface(titleSurface, NULL, screen, &destRect);
        TTF_CloseFont(titleFont);

        // Display pagination page number / total_pages at the bottom
        std::string pageInfo = std::to_string(currentPage) + " / " + std::to_string(total_pages);
        SDL_Surface* textSurface = TTF_RenderText_Blended(font, pageInfo.c_str(), {255,255,255});
        destRect = {100, 445, 0, 0};  // Positon for page counter
        SDL_BlitSurface(textSurface, NULL, screen, &destRect);
    }

    //  Close the customFont
    if (!customFontPath.empty()) {
        TTF_CloseFont(currentFont);
    }

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
    }
}



bool Menu::isRomListMenu() const {
    for (const auto& item : items) {
        if (auto simpleItem = dynamic_cast<SimpleMenuItem*>(item.get())) {
            if (!simpleItem->getPath().empty()) {
                return true;
            }
        }
    }
    return false;
}

void Menu::setSelectionRectangleProperties(const SDL_Color& color, int width, int height) {
    selectionRectangleColor = color;
    selectionRectangleWidth = width;
    selectionRectangleHeight = height;
}

void Menu::enableSelectionRectangle(bool enable) {
    drawSelectionRectangle = enable;
}

SystemMenu::SystemMenu(std::string backgroundPath, std::string settingsFont) {

    this->itemsPerPage = this->cfg.getIntValue(SettingId::SETTINGS_PER_PAGE);

    setBackground(backgroundPath);  // Assuming Menu has this method. If not, you might need to adapt.

    setFont(settingsFont, 32);
    setItemPosition(10,92);
    setSpacing(46);
    useSelectionRectangle = true;
    // FIXME, width, line height, etc. needs to be set as options
    setSelectionRectangleProperties({0x10, 0x22, 0xa0, 128}, 640, 46); // Semi-transparent red rectangle with width 200px and height 24px

}

RomMenu::RomMenu() {
    // FIXME decide how to deal with rom ids to know which rom they belong to
    addItem(std::make_unique<BooleanMenuItem>(SettingId::None, //"Rom.autostart", 
                                              "AUTOSTART", 
                                              "OFF"));
    
    std::vector<std::string> coreOptions = {"mame2003_plus", "fbneo", "fbalpha", "mame2000", "mame2010"};

    addItem(std::make_unique<MultiOptionMenuItem>(SettingId::None, //"Rom.selectCore", 
                                                  "SELECT CORE/EMULATOR", 
                                                  coreOptions[0], 
                                                  coreOptions));
    addItem(std::make_unique<BooleanMenuItem>(SettingId::None, //"Rom.saveState",
                                              "AUTO SAVE STATE", 
                                              "OFF"));
    addItem(std::make_unique<BooleanMenuItem>(SettingId::None, //"Rom.loadState", 
                                              "LOAD STATE", 
                                              "OFF"));

    std::vector<std::string> overclockValues = {"OFF", "840 MHz", "1008 MHz", "1296 MHz"};

    addItem(std::make_unique<MultiOptionMenuItem>(SettingId::None, //"Rom.overclock",
                                                  "ROM OVERCLOCK", 
                                                  overclockValues[0], overclockValues));


    std::string backgroundPath = this->cfg.getValue(SettingId::HOME_PATH) 
                                 + ".simplemenu/resources/rom_settings.png";
    setBackground(backgroundPath);  // Assuming Menu has this method. If not, you might need to adapt.

    std::string settingsFont = this->cfg.getValue(SettingId::HOME_PATH) 
                               + ".simplemenu/resources/Akrobat-Bold.ttf";
    setFont(settingsFont, 32);
    setItemPosition(10,92);
    setSpacing(46);
    useSelectionRectangle = true;
    // FIXME, width, line height, etc. needs to be set as options
    setSelectionRectangleProperties({0, 0, 0, 200}, 640, 46); // Semi-transparent red rectangle with width 200px and height 24px
}

bool RomMenu::isRomMenu() const {
    return true;
}

void Menu::setBackground(const std::string& backgroundPath) {
    std::cout << "Setting background: " << backgroundPath << std::endl;
    SDL_Surface* newBackground = IMG_Load(backgroundPath.c_str());
    if (newBackground) {
        if (background) {
            SDL_FreeSurface(background);
        }
        background = newBackground;
    } else {
        // Handle error: could not load the background image.
    }
}

void Menu::setFont(const std::string& fontPath, int fontSize) {
    customFontPath = fontPath;
    customFontSize = fontSize;
}

void Menu::setItemPosition(int x, int y) {
    itemOffset_x = x;
    itemOffset_y = y;
}

void Menu::setSpacing(int spacing) {
    customSpacing = spacing;
}
