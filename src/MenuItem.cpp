#include "MenuItem.h"
#include "Menu.h"
#include "State.h"
#include "Application.h"
#include "Exception.h"

#include <iostream>
#include <filesystem>
#include <memory>
#include <fstream>

std::unordered_map<std::string, SDL_Surface*> SimpleMenuItem::thumbnailCache;

std::string MenuItem::getTitle() {
    return this->title;
}

std::string MenuItem::getValue() {
    return this->value;
}

std::string MenuItem::getRomAlias() {
    return this->alias;
}

void MenuItem::getAlias() {
    std::string displayTitle = title;
    std::filesystem::path romPath(title);
    std::string filenameWithoutExt = romPath.stem().string();
    // Check if the rom name exists in the alias map
    if (aliasMap.find(filenameWithoutExt) != aliasMap.end()) {
        displayTitle = aliasMap[filenameWithoutExt];
    } else {
        // If no alias is available, remove the file extension
        displayTitle = filenameWithoutExt;
    }

    alias = displayTitle;
}

  /**
   * The subscription management methods.
   */
  void MenuItem::attach(ISettingsObserver *observer) {
    this->observers_.push_back(observer);
    std::cout << "Observer attached.\n";
  }

  void MenuItem::detach(ISettingsObserver *observer) {
    this->observers_.remove(observer);
  }

  void MenuItem::notifySettingsChange() {
    std::list<ISettingsObserver *>::iterator iterator = 
        this->observers_.begin();

    while (iterator != this->observers_.end()) {
        (*iterator)->settingsChanged(this->id, this->value);
        ++iterator;
        std::cout << "Observer notified: " << this->id << " : " 
            << this->value << "\n";
    }
  }

Menu* MenuItem::getRootMenu() const {
    return rootMenu ? rootMenu : (parentMenu ? parentMenu->getRootMenu() : nullptr);
}

  /**
   * SimpleMenuItem methods 
   */

void SimpleMenuItem::executeAction() {
    std::cout << "executeAction for entry " << title << " value: " << value << std::endl;
    // Execute the action associated with this menu item
    // This can be a callback or some other function
    if (title == "Back") {
    // This is a simplistic way to handle it. 
    // A better way would be to use callbacks or command patterns.
        Application::getInstance()->showMainMenu();
    } else if (title == "Return to Main Menu") {
        Application::getInstance()->showMainMenu();
    } else if (!path.empty()) {
        std::cout << "Launching rom: " << path << " title: " << title << std::endl;
    
        std::map<std::string, ConsoleData> consoleDataMap = Configuration::getInstance().parseIniFile("/userdata/system/.simplemenu/section_groups/" + this->getRootMenu()->getTitle());

        // Get the title of the parent menu
        std::string parentTitle = this->getParentMenu()->getTitle();

        std::cout << "parenTitle " << parentTitle << std::endl;

        // FIXME: temporary solution to select the first available emulator launcher as defined
        //        in the .ini execs. This will have to be changed once the launcher is selected and saved
        std::string execLauncher;

        // Check if the parentTitle exists in the consoleDataMap
        if (consoleDataMap.find(parentTitle) != consoleDataMap.end()) {
            // Access the ConsoleData for the parentTitle
            ConsoleData consoleData = consoleDataMap[parentTitle];

            // Check if the execs vector is not empty
            if (!consoleData.execs.empty()) {
                // Retrieve the first exec string
                execLauncher = consoleData.execs.front();
            }
        }

        // SaveState currentState;
        // currentState.menuName = "mainMenu"; // Get the current menu's name or ID
        // currentState.selectedItemPosition = 5; // Get the current selected item's position
        State::getInstance()->saveAppState("/userdata/system/simplermenu_plus/state.txt");

        // Launch emulator
        std::string command = execLauncher + " '" + path + "'";
        std::cout << "Executing: " << command << std::endl;

        system(command.c_str());

        // Exit the application to free all resources
        SDL_Quit();
        exit(0);
    } else if (title == "QUIT") {
        SDL_Quit();
        exit(0);
    } else if (title == "Save") {
        // TODO this should be notified to Application in a cleaner way
        //      This class should not be aware of the actions to perform
        this->notifySettingsChange();
    }
}

void SimpleMenuItem::loadThumbnail() {
    // If thumbnail is already in cache, set it and return
    if (thumbnailCache.find(thumbnailPath) != thumbnailCache.end()) {
        thumbnail = thumbnailCache[thumbnailPath];
        return;
    }

    // If thumbnail exists, proceed with loading and caching
    if (thumbnailExists()) { 
        tmpThumbnail = IMG_Load(thumbnailPath.c_str());

        int thumbnailWidth = theme.getIntValue("GENERAL.art_max_w");
        int thumbnailHeight = theme.getIntValue("GENERAL.art_max_h");

        // Check if the thumbnail needs to be resized
        if (tmpThumbnail->w != thumbnailWidth || tmpThumbnail->h != thumbnailHeight) {
            double scaleX = (double)thumbnailWidth / tmpThumbnail->w;
            double scaleY = (double)thumbnailHeight / tmpThumbnail->h;
            double scale = std::min(scaleX, scaleY);

            SDL_Surface* resizedThumbnail = zoomSurface(tmpThumbnail, scale, scale, SMOOTHING_ON);
            
            // Free the original loaded thumbnail as it's no longer needed
            if (tmpThumbnail) {
                SDL_FreeSurface(tmpThumbnail);
            }

            tmpThumbnail = resizedThumbnail;
        }

        // Before caching the new thumbnail, free any existing surface associated with that thumbnailPath
        if (thumbnailCache.find(thumbnailPath) != thumbnailCache.end()) {
            //SDL_FreeSurface(thumbnailCache[thumbnailPath]);
            thumbnailCache.erase(thumbnailPath);
        }

        // Cache and set the thumbnail
        thumbnailCache[thumbnailPath] = tmpThumbnail;
        thumbnail = tmpThumbnail;
    }
}

std::unordered_map<std::string, std::string> MenuItem::aliasMap;

void MenuItem::loadAliases() {
    std::ifstream infile(Configuration::getInstance().getValue(SettingId::ALIAS_PATH));
    std::string line;
    while (std::getline(infile, line)) {
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string filename = line.substr(0, pos);
            std::string alias = line.substr(pos + 1);
            aliasMap[filename] = alias;
        }
    }
}

void SimpleMenuItem::render(SDL_Surface* screen, TTF_Font* font, int x, int y, bool isSelected, MenuState currentState) {
    // If we have a section or system menu that is basically a background picture
    if((currentState == MenuState::SYSTEMS_MENU || currentState == MenuState::SECTIONS_MENU)) {
        if(this->background == nullptr) {
            determineAndSetBackground(screen, currentState);
        }

        if (currentBackground == nullptr) {
            // If there's no background, render the folder name centered with black background
            if (isSelected) {
                //SDL_Rect dstRect;
                int x = (screen->w) / 2;
                int y = (screen->h) / 2;
                SDL_FillRect(screen, nullptr, SDL_MapRGB(screen->format, 0, 0, 0));  // Filling with black
                RenderUtils::getInstance()->renderText(screen, "generalFont", title, x, y, 0, 0, {255, 255, 255}, 1);
            }
            return;
        } else {
                SDL_BlitSurface(background, NULL, screen, NULL);
        }
    } else {

        // Otherwise, we are in a romlist, system settings or rom settings view
        // In this case we render a list of items:
        // * For Romlist we also render a thumbnail view
        // * For System or Rom Settings we render the title and value
        SDL_Color textColor = isSelected ? theme.getColor("DEFAULT.selected_item_font_color"):theme.getColor("DEFAULT.items_font_color"); 

        // Determine text width
        SDL_Surface* textSurface = TTF_RenderText_Blended(font, alias.c_str(), textColor);
        titleWidth = textSurface->w;

        // TODO replace clipWidth the correct width based on theme.ini settings
        int clipWidth = theme.getIntValue("GENERAL.game_list_w"); 
        if( currentState == MenuState::SYSTEM_SETTINGS_MENU || currentState == MenuState::ROM_SETTINGS_MENU) {
            clipWidth = 500; // FIXME: default for setting list so it does no overlap with value
        }

        // Create the scrolling view for titles that are too wide
        if (isSelected && SDL_GetTicks() - selectTime > SCROLL_TIMEOUT) {
            if (scrollPixelPosition < titleWidth - clipWidth) { 
                scrollPixelPosition += 1;  // Increment by 1 pixel. Adjust for faster scrolling.
                if (scrollPixelPosition == titleWidth - clipWidth) {
                    // Record the time when scrolling completes
                    scrollEndTime = SDL_GetTicks();
                }
            } else if (SDL_GetTicks() - scrollEndTime > END_SCROLL_PAUSE) {
                // Reset the scroll position after the timeout period has elapsed
                scrollPixelPosition = 0;
                selectTime = SDL_GetTicks();
            }
        }

        SDL_Rect destRect = {static_cast<Sint16>(x - scrollPixelPosition), static_cast<Sint16>(y), 0, 0};  // Adjust x position by scrollPixelPosition
        SDL_Rect clipRect = {static_cast<Sint16>(x), static_cast<Sint16>(y), clipWidth, static_cast<Uint16>(textSurface->h)}; // Ensure text doesn't spill over the intended area

        SDL_SetClipRect(screen, &clipRect);
        SDL_BlitSurface(textSurface, nullptr, screen, &destRect);
        SDL_SetClipRect(screen, NULL);  // Reset the clip rect

        // If we have a value associated with the item (e.g. settings) we render that at the end of each row
        if (value != "") {
            // Render the value to the right of the title
            SDL_Surface* valueSurface = TTF_RenderText_Blended(font, value.c_str(), textColor);
        
            // Position the value surface to the right of the title
            SDL_Rect valueDestRect = {static_cast<Sint16>(cfg.getIntValue(SettingId::SCREEN_WIDTH) - valueSurface->w - 10), static_cast<Sint16>(y), 0, 0};
            SDL_BlitSurface(valueSurface, nullptr, screen, &valueDestRect);
            SDL_FreeSurface(valueSurface);
        }

        // Load Thumbnail for the selected Rom
        // TODO replace coordinates from those in the theme.ini
        if (isSelected) {
            if(!thumbnail) {
                loadThumbnail();
            }
            Uint16 x = theme.getIntValue("GENERAL.art_x"); 
            Uint16 y = theme.getIntValue("GENERAL.art_y"); 
            Uint16 w = theme.getIntValue("GENERAL.art_max_w"); 
            Uint16 h = theme.getIntValue("GENERAL.art_max_h"); 
            //SDL_Rect destRect = {static_cast<Sint16>(screen->w / 2 - 20), 100, 0, 0};
            SDL_Rect destRect = {x, y, w, h};
            SDL_BlitSurface(thumbnail, nullptr, screen, &destRect);
        }

        SDL_FreeSurface(textSurface);
    }

    // Some themes have text in the section selection
    if (theme.getValue("GENERAL.display_section_group_name") == "1" && currentState == MenuState::SECTIONS_MENU) {
        SDL_Color white = {255, 255, 255};

        // Remove extension from section and transform to uppercase
        std::filesystem::path ss(title);
        std::string sectionName(ss.stem().string()); 
        transform(sectionName.begin(), sectionName.end(), sectionName.begin(), ::toupper);

        int sectionFontSize = 96;
        if(Configuration::getInstance().getIntValue(SettingId::SCREEN_WIDTH) == 320) {
            sectionFontSize = 48;
        }

        SDL_Surface* folderNameSurface = renderText(sectionName, white, theme.getValue("GENERAL.font", true), sectionFontSize);
        if (folderNameSurface && isSelected) {
            SDL_Rect dstRect;
            dstRect.x = (screen->w - folderNameSurface->w) / 2;
            dstRect.y = (screen->h - folderNameSurface->h) / 2;
            dstRect.w = folderNameSurface->w;
            dstRect.h = folderNameSurface->h;

            // Create a semi-transparent surface for the background
            SDL_Surface* transparentBg = SDL_CreateRGBSurface(0, cfg.getIntValue(SettingId::SCREEN_WIDTH), dstRect.h, 32, 0, 0, 0, 0);

            // Enable blending for the surface
            SDL_SetAlpha(transparentBg, SDL_SRCALPHA, 127);

            SDL_FillRect(transparentBg, NULL, SDL_MapRGBA(transparentBg->format, 0, 0, 0, 10)); // Fill with black color and 50% opacity

            SDL_Rect fadeRect = {0, getConfig().getIntValue(SettingId::SCREEN_HEIGHT) / 2 - folderNameSurface->h / 2, 
                                 getConfig().getIntValue(SettingId::SCREEN_WIDTH), 
                                 getConfig().getIntValue(SettingId::SCREEN_HEIGHT) / 2 + folderNameSurface->h / 2};

            // Render the semi-transparent background
            SDL_BlitSurface(transparentBg, NULL, screen, &fadeRect);

            // Render the text on top of the semi-transparent background
            SDL_BlitSurface(folderNameSurface, NULL, screen, &dstRect);

            SDL_FreeSurface(folderNameSurface);
            SDL_FreeSurface(transparentBg); // Don't forget to free the surface when done
        }
    }

    // Add the game count field for the system view if display_game_count is enabled
    if (currentState == MenuState::SYSTEMS_MENU && theme.getIntValue("GENERAL.display_game_count") == 1 ) {            
            RenderUtils::getInstance()->renderText(screen, "textFont", 
                                                   std::to_string(numberOfItems) + " GAMES", 
                                                   theme.getIntValue("GENERAL.game_count_x"), 
                                                   theme.getIntValue("GENERAL.game_count_y"), 0, 0, 
                                                   theme.getColor("GENERAL.game_count_font_color"), 1); 

            return;
    }
}

std::string SimpleMenuItem::getName() const {
    return title;
}

bool SimpleMenuItem::thumbnailExists() {//const std::string& romPath) {
    return std::filesystem::exists(thumbnailPath);
}

void SimpleMenuItem::select() {
    scrollPixelPosition = 0;
    selectTime = SDL_GetTicks();
    
    // Measure the width of the title
    TTF_Font* currentFont = Application::getInstance()->font;
    TTF_SizeText(currentFont, title.c_str(), &titleWidth, nullptr);
    
    if (!thumbnail && thumbnailExists()) {
        loadThumbnail();
    }     
}

void SimpleMenuItem::deselect() {
    thumbnail = nullptr;
    scrollPixelPosition = 0;
}

SDL_Surface* MenuItem::renderText(const std::string& text, SDL_Color color, std::string fontType, int fontSize) {
    TTF_Font* font = TTF_OpenFont(fontType.c_str(), fontSize);  // Adjust font path and size as necessary
    if (!font) {
        // Handle error
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return nullptr;
    }
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, text.c_str(), color);
    TTF_CloseFont(font);
    return textSurface;
}

std::string MenuItem::getFolderName() const {
    if (hasSubMenu()) {
        // If it has a submenu, it's a folder itself
        return title;
    } else {
        // If not, it's a ROM, so we get the parent directory's name from the path
        std::filesystem::path romPath(dynamic_cast<const SimpleMenuItem*>(this)->getPath());
        return romPath.parent_path().filename().string();
    }
}

void SimpleMenuItem::determineAndSetBackground(SDL_Surface* screen, MenuState currentState) {
    std::string backgroundPath;
    if(currentState == MenuState::SECTIONS_MENU) {
        std::string title = this->getTitle();
        size_t lastindex = title.find_last_of(".");
        std::string titleName = title.substr(0,lastindex);
        backgroundPath = Configuration::getInstance().getThemePath() + "resources/section_groups/" + titleName + ".png";
    } else if(currentState == MenuState::SYSTEMS_MENU) {
        backgroundPath = Configuration::getInstance().getThemePath() + theme.getValue(this->getFolderName() + ".logo");
    } else if(currentState == MenuState::ROMLIST_MENU) {
        backgroundPath = Configuration::getInstance().getThemePath() + theme.getValue("DEFAULT.background");
    }
    std::cout <<"MenuItem SetBackground " << backgroundPath << std::endl;
    if (background) {
        SDL_FreeSurface(background);
        background = nullptr;
    }

    std::cout << "rom title: " << title << std::endl;
    background = IMG_Load(backgroundPath.c_str());
    if (!background) {
        std::cerr << "Failed to load background: " << IMG_GetError() << std::endl;
    }

    SDL_BlitSurface(background, NULL, screen, NULL);

    currentBackground = background;
    //return this->background;
}

bool BooleanMenuItem::getValue() const {
    return boolValue;
}

void BooleanMenuItem::setValue(bool newValue) {
    boolValue = newValue;
    value = boolValue ? "ON" : "OFF";
    this->notifySettingsChange();
}

void BooleanMenuItem::toggleValue() {
    setValue(!boolValue);
    std::cout << "Value set to " << value << std::endl;
    this->notifySettingsChange();
}

void BooleanMenuItem::navigateLeft() {
    std::cout << "boolnavleft: " << std::endl;

    toggleValue();
}

void BooleanMenuItem::navigateRight() {
    std::cout << "boolnavright: " << std::endl;

    toggleValue();
}

MultiOptionMenuItem::MultiOptionMenuItem(
    const SettingId& id,
    const std::string& title,
    const std::string& value,
    const std::vector<std::string>& availableOptions) : 
    SimpleMenuItem(id, 
                   title, 
                   "", 
                   "") {
        this->options = availableOptions;
        this->setValue(value);
    }

int MultiOptionMenuItem::mod(int a, int b) { 
    return (a % b + b) % b; 
}

void MultiOptionMenuItem::setValue(const std::string& value) {

    int valueIndex = -1;

    for (int i=0; i < this->options.size(); i++) {
        if (this->options[i] == value) {
            valueIndex = i;
            break;
        }
    }

    if (valueIndex >= 0) {
        this->currentIndex = valueIndex;
        this->value = this->options[valueIndex];
    } else {
        std::string optionsStr;
        for (const auto &option : this->options) optionsStr += option + " ";
            throw OptionValueNotFoundException(
                "Value " + value + " not found in " + optionsStr);
    }
}

void MultiOptionMenuItem::navigateLeft() {
    this->currentIndex = this->mod((this->currentIndex - 1), this->options.size());
    this->value = this->options[this->currentIndex];
}

void MultiOptionMenuItem::navigateRight() {
    this->currentIndex = this->mod((this->currentIndex + 1), this->options.size());
    this->value = this->options[this->currentIndex];
}

IntegerMenuItem::IntegerMenuItem(const SettingId& id,
                                 const std::string& name, 
                                 const std::string& value, 
                                 int min, 
                                 int max)
                    : SimpleMenuItem(id, name, "", value) {
    this->intValue = std::stoi(value);
    this->maxValue = max;
    this->minValue = min;
}

void IntegerMenuItem::updateValuefromInt() {
    this->value = std::to_string(this->intValue);
    this->notifySettingsChange();
}

void IntegerMenuItem::navigateLeft() {
    if (this->intValue > (this->minValue + 5)) {
        this->intValue -= 5;
        this->updateValuefromInt();
    }
}

void IntegerMenuItem::navigateRight() {
    if (this->intValue < (this->maxValue - 5)) {
        this->intValue += 5;
        this->updateValuefromInt();
    }
    
}

