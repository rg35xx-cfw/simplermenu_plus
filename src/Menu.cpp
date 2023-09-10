#include "Menu.h"
#include "State.h"
#include "Application.h"

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
    if(currentState == MenuState::SYSTEMS_MENU) {
        MenuItem* selectedItem = items[selectedItemIndex].get();
        background = selectedItem->getBackground();     
    } else if(currentState == MenuState::SYSTEM_SETTINGS_MENU || currentState == MenuState::ROM_SETTINGS_MENU) {
        background = getBackground();   
    }

    if (background) {
        SDL_BlitSurface(background, NULL, screen, NULL);
    } else {
        // If there's no background, we set the background to black
        SDL_FillRect(screen, nullptr, SDL_MapRGB(screen->format, 0, 0, 0));
    }

    // 3. Render each menu item
    int x = customFontPath.empty() ? 
        theme.getIntValue("GENERAL.game_list_x") : itemOffset_x;
    int y = customFontPath.empty() ? 
        theme.getIntValue("GENERAL.game_list_y") : itemOffset_y;

    // Open customFont if defined (used for settings menu)
    TTF_Font* currentFont = customFontPath.empty() ? font : TTF_OpenFont(customFontPath.c_str(), customFontSize);

    int spacing = customSpacing ? customSpacing : theme.getIntValue("GENERAL.items_separation");

    if (useSelectionRectangle || (currentState == MenuState::ROMLIST_MENU)) {
        SDL_Rect rectangle;
        if (currentState == MenuState::ROMLIST_MENU) {
            rectangle.x = x - 2;
            rectangle.y = y + (selectedItemIndex - startIndex) * spacing; // Adjust for current item position
        } else {
            rectangle.x = x - 10;
            rectangle.y = y - 5 + (selectedItemIndex - startIndex) * spacing; // Adjust for current item position
        }
        rectangle.w = (selectionRectangleWidth > 0) ? selectionRectangleWidth : theme.getIntValue("GENERAL.game_list_w") + 4; // screen width as fallback
        rectangle.h = (selectionRectangleHeight > -0) ? selectionRectangleHeight : theme.getIntValue("GENERAL.items_separation");

        // TODO: decide if the same color is going to be used for settings or just for the romlist
        if (currentState == MenuState::ROMLIST_MENU) {
            selectionRectangleColor = theme.getColor("DEFAULT.selected_item_background_color");
        }

        SDL_FillRect(screen, &rectangle, SDL_MapRGB(screen->format, selectionRectangleColor.r, selectionRectangleColor.g, selectionRectangleColor.b));
    }

    int endIndex = std::min(startIndex + itemsPerPage, static_cast<int>(items.size()));
    for (int i = startIndex; i < endIndex; i++) {
        bool isSelected = (i == selectedItemIndex);
        int spacing = customSpacing ? customSpacing : theme.getIntValue("GENERAL.items_separation");// 24;
        // We only render the current item if we are in single screen view, otherwise we display all items
        if (isSelected || (currentState != MenuState::SYSTEMS_MENU && currentState != MenuState::SECTIONS_MENU)) {
            items[i]->render(screen, currentFont, x, y + (i - startIndex) * spacing, isSelected, currentState);
        }
    }

    // If we are in the rom list view, we render the number of pages as well as the system title
    if (currentState == MenuState::ROMLIST_MENU) {

        MenuItem* selectedItem = items[selectedItemIndex].get();

        std::string systemTitle = selectedItem->getFolderName();
        transform(systemTitle.begin(), systemTitle.end(), systemTitle.begin(), ::toupper);
        std::string fontPath = theme.getValue("GENERAL.textX_font", true);
//        TTF_Font* titleFont = TTF_OpenFont(fontPath.c_str(), 32); // FIXME: size needs to be based on theme settings

        //RenderUtils renderUtil(titleFont);
        //renderUtil.setFont(theme.getValue("GENERAL.textX_font", true),  theme.getIntValue("GENERAL.text1_font_size"));

        RenderUtils::getInstance()->renderText(screen, "generalFont", systemTitle, theme.getIntValue("GENERAL.text1_x"), theme.getIntValue("GENERAL.text1_y"), 0, 0, {255, 255, 255}, theme.getIntValue("GENERAL.text1_alignment"));

        // // Add rom/art title (some graphic themes hide this rendering it outside the screen)
  //      titleFont = TTF_OpenFont(fontPath.c_str(), theme.getIntValue("GENERAL.art_text_font_size"));

        // RenderUtils::getInstance().setFont(fontPath, theme.getIntValue("GENERAL.art_text_font_size"));
        int x = theme.getIntValue("GENERAL.art_x") + theme.getIntValue("GENERAL.art_max_w")/2;
        int y = theme.getIntValue("GENERAL.art_y") + 
                theme.getIntValue("GENERAL.art_max_h") + 
                theme.getIntValue("GENERAL.art_text_distance_from_picture") +
                theme.getIntValue("GENERAL.art_text_line_separation");
        RenderUtils::getInstance()->renderText(screen,"generalFont", items[selectedItemIndex]->getRomAlias(), x, y, 0, 0, {255, 255, 255}, 1);

        // Calculate number of pages FIXME: move that to the constructor
        total_pages = (items.size() + itemsPerPage - 1)/ itemsPerPage;

        // Display pagination page number / total_pages at the bottom
        std::string pageInfo = std::to_string(currentPage + 1) + " / " + std::to_string(total_pages);
        x = theme.getIntValue("GENERAL.text2_x");
        y = theme.getIntValue("GENERAL.text2_y");
        //renderUtil.setFont(fontPath, theme.getIntValue("GENERAL.text2_font_size"));

        RenderUtils::getInstance()->renderText(screen, "generalFont", pageInfo, x, y, 0, 0, {255, 255, 255}, theme.getIntValue("GENERAL.text2_alignment"));
    }

    if (currentState == MenuState::ROM_SETTINGS_MENU) {
        //std::string fontPath = theme.getValue("GENERAL.textX_font", true);
        //TTF_Font* titleFont = TTF_OpenFont(fontPath.c_str(), 32); // FIXME: size needs to be based on theme settings

        //RenderUtils renderUtil(titleFont);
        // std::string settingsFont = 
        // this->cfg.getValue(SettingId::HOME_PATH) 
        // + ".simplemenu/resources/Akrobat-Bold.ttf";

        //renderUtil.setFont(settingsFont,  32);

        SDL_Color textColor = theme.getColor("DEFAULT.selected_item_font_color");

        RenderUtils::getInstance()->renderText(screen, "textFont", getTitle(), Configuration::getInstance().getIntValue(SettingId::SCREEN_WIDTH) / 2 , 65, 0, 0, textColor, 1);

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

SystemSettingsMenu::SystemSettingsMenu(std::string backgroundPath, std::string settingsFont) : Menu("System Settings Menu") {

    this->itemsPerPage = Configuration::getInstance().getIntValue(SettingId::SETTINGS_PER_PAGE);

    setBackground(backgroundPath);  // Assuming Menu has this method. If not, you might need to adapt.
    background = IMG_Load(backgroundPath.c_str());
    if (!background) {
        std::cerr << "Failed to load Settings background: " << IMG_GetError() << std::endl;
    }

    setFont(settingsFont, 32);
    setItemPosition(10,92);
    setSpacing(46);
    useSelectionRectangle = true;
    // FIXME, width, line height, etc. needs to be set as options
    setSelectionRectangleProperties({0x10, 0x22, 0xa0, 128}, 640, 46); // Semi-transparent red rectangle with width 200px and height 24px
}

RomSettingsMenu::RomSettingsMenu(std::string backgroundPath, std::string settingsFont) : Menu("Rom Settings Menu") {

    this->itemsPerPage = Configuration::getInstance().getIntValue(SettingId::SETTINGS_PER_PAGE);

    setBackground(backgroundPath);  // Assuming Menu has this method. If not, you might need to adapt.
    background = IMG_Load(backgroundPath.c_str());
    if (!background) {
        std::cerr << "Failed to load Rom Settings background: " << IMG_GetError() << std::endl;
    }

    setFont(settingsFont, 32);
    setItemPosition(10,92);
    setSpacing(46);
    useSelectionRectangle = true;
    // FIXME, width, line height, etc. needs to be set as options
    setSelectionRectangleProperties({0, 0, 0, 200}, 640, 46); // Semi-transparent red rectangle with width 200px and height 24px
}

bool RomSettingsMenu::isRomMenu() const {
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
    SDL_FreeSurface(newBackground);
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

std::string extractBeforeUnderscore(const std::string& str) {
    size_t lastSlash = str.find_last_of("/");
    size_t underscorePos = str.find_last_of("_");

    // If there's no '_', return the original string
    if (underscorePos == std::string::npos) {
        return str;
    }

    // If there's no '/', start from the beginning of the string
    if (lastSlash == std::string::npos) {
        return str.substr(0, underscorePos);
    }

    // Extract the portion between the last slash and the underscore
    return str.substr(lastSlash + 1, underscorePos - lastSlash - 1);
}

void RomSettingsMenu::populateForROM(MenuItem* rom) {

    setTitle(rom->getRomAlias());

    //std::cout << "ROM --- " << rom->getFolderName() << " --- " << rom->getRootMenu()->getTitle() <<std::endl;

    // Clear existing items
    this->clearItems();

    if (hasCustomSettings(rom)) {
        // populate the menu with custom settings
    } else {
        // populate the menu with default settings
        // FIXME decide how to deal with rom ids to know which rom they belong to
        addItem(std::make_unique<BooleanMenuItem>(SettingId::None, //"Rom.autostart", 
                                                "AUTOSTART", 
                                                "OFF"));
        
        std::string iniFile = "/userdata/system/.simplemenu/section_groups/" + rom->getRootMenu()->getTitle();

        std::map<std::string, ConsoleData> consoleDataMap = Configuration::getInstance().parseIniFile(iniFile);

        // Get the title of the parent menu
        std::string parentTitle = rom->getParentMenu()->getTitle();

        std::cout << "parenTitle " << parentTitle << std::endl;

        // FIXME: temporary solution to select the first available emulator launcher as defined
        //        in the .ini execs. This will have to be changed once the launcher is selected and saved
        ConsoleData consoleData = consoleDataMap[parentTitle];

        std::vector<std::string> coreOptions = consoleData.execs;
        std::vector<std::string> coreList;

        for (const auto& str : coreOptions) {
            std::string out = extractBeforeUnderscore(str);
            coreList.push_back(out);
        }

        std::cout << "execs " << coreOptions[0] << " -> " << coreList[0] << std::endl;

        addItem(std::make_unique<MultiOptionMenuItem>(SettingId::None, //"Rom.selectCore", 
                                                    "SELECT CORE/EMULATOR", 
                                                    coreList[0], 
                                                    coreList));
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
    }

    // Add other common menu items
}