#include "MenuItem.h"
#include "Menu.h"
#include "State.h"
#include "Application.h"

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

void MenuItem::setBackground(const std::string& backgroundPath, SDL_Surface* screen) {
    std::cout <<"MenuItem SetBackground " << backgroundPath << std::endl;
    if (background) {
        SDL_FreeSurface(background);
        background = nullptr;
    }
    background = IMG_Load(backgroundPath.c_str());
    if (!background) {
        std::cerr << "Failed to load background: " << IMG_GetError() << std::endl;
    }
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
      (*iterator)->settingsChanged(this->title, this->value);
      ++iterator;
      std::cout << "Observer notified.\n";
    }
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
        Application::getInstance().showMainMenu();
    } else if (title == "Return to Main Menu") {
        Application::getInstance().showMainMenu();
    } else if (!path.empty()) {
        std::cout << "Launching rom: " << path << " title: " << title << std::endl;
    
        // Launch emulator
        std::string command = "/userdata/system/.simplemenu/resources/launcher.sh '" + path + "'";
        std::cout << "Executing: " << command << std::endl;

        system(command.c_str());

        // Exit the application to free all resources
        exit(0);
    } else if (title == "QUIT") {
        exit(0);
    }
}

SDL_Surface* SimpleMenuItem::loadThumbnail() {
    if (thumbnailCache.find(thumbnailPath) != thumbnailCache.end()) {
        // Return from cache
        return thumbnailCache[thumbnailPath];
    }

    if (thumbnailExists()) { // Uses the member variable thumbnailPath
        // Load and cache
        SDL_Surface* thumbnail = IMG_Load(thumbnailPath.c_str());
        int thumbnailWidth = Configuration::getInstance().getIntValue("Menu.thumbnailWidth");
        int thumbnailHeight = Configuration::getInstance().getIntValue("Menu.thumbnailHeight");

        if (thumbnail->w > thumbnailWidth || thumbnail->h > thumbnailHeight) {
            double scaleX = (double)thumbnailWidth / thumbnail->w;
            double scaleY = (double)thumbnailHeight / thumbnail->h;
            double scale = std::min(scaleX,scaleY);

            thumbnail = zoomSurface(thumbnail, scale, scale, SMOOTHING_ON);
        }

        thumbnailCache[thumbnailPath] = thumbnail;
        return thumbnail;
    }

    return nullptr;
}

std::unordered_map<std::string, std::string> SimpleMenuItem::aliasMap;

void SimpleMenuItem::loadAliases() {
    std::ifstream infile(Configuration::getInstance().getValue("Menu.aliasPath"));
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
    if(currentState == MenuState::SYSTEMS_MENU) {
        SDL_Surface* currentBackground = this->getAssociatedBackground();  // Get the background from the current MenuItem
        if (!currentBackground) {
            // If there's no background, render the folder name centered
            SDL_Color white = {255, 255, 255};
            SDL_Surface* folderNameSurface = renderText(title, white);
            if (folderNameSurface && isSelected) {
                SDL_Rect dstRect;
                dstRect.x = (screen->w - folderNameSurface->w) / 2;
                dstRect.y = (screen->h - folderNameSurface->h) / 2;
                dstRect.w = folderNameSurface->w;
                dstRect.h = folderNameSurface->h;
                SDL_FillRect(screen, nullptr, SDL_MapRGB(screen->format, 0, 0, 0));  // Filling with black

                SDL_BlitSurface(folderNameSurface, NULL, screen, &dstRect);
                SDL_FreeSurface(folderNameSurface);
            }
            return;
        }
    } else {
        // Set the colors, white for selected text, gray for non-selected text
        SDL_Color textColor = isSelected ? SDL_Color{0xe7,0xcb,0x08}: SDL_Color{180, 180, 180}; 

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

        SDL_Surface* textSurface = TTF_RenderText_Blended(font, displayTitle.c_str(), textColor);
        titleWidth = textSurface->w;

        if (isSelected && SDL_GetTicks() - selectTime > SCROLL_TIMEOUT) {
            if (scrollPixelPosition < titleWidth - 220) { // FIXME: calculate width dynamically based on theme
                scrollPixelPosition += 1;  // Increment by 1 pixel. Adjust for faster scrolling.
                if (scrollPixelPosition == titleWidth - 220) {
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
        SDL_Rect clipRect = {static_cast<Sint16>(x), static_cast<Sint16>(y), 220 /*screen->w / 2 - 20*/, static_cast<Uint16>(textSurface->h)}; // Ensure text doesn't spill over the intended area

        SDL_SetClipRect(screen, &clipRect);
        SDL_BlitSurface(textSurface, nullptr, screen, &destRect);
        SDL_SetClipRect(screen, NULL);  // Reset the clip rect

        if (value != "") {
            // Render the value to the right of the title
            //TTF_Font* titleFont = TTF_OpenFont(Configuration::getInstance().getValue("Menu.MainFont").c_str(), 32);
            SDL_Surface* valueSurface = TTF_RenderText_Blended(font, value.c_str(), textColor);
        
            // Position the value surface to the right of the title
            SDL_Rect valueDestRect = {static_cast<Sint16>(640 - valueSurface->w - 10), static_cast<Sint16>(y), 0, 0};
            SDL_BlitSurface(valueSurface, nullptr, screen, &valueDestRect);
            SDL_FreeSurface(valueSurface);
        }

        // Load Thumbnail for the selected Rom
        if (isSelected) {
            SDL_Surface* thumbnail = loadThumbnail();
            if (thumbnail) {
                SDL_Rect destRect = {static_cast<Sint16>(screen->w / 2 - 20), 100, 0, 0};
                SDL_BlitSurface(thumbnail, nullptr, screen, &destRect);
            }
        }

        SDL_FreeSurface(textSurface);
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
    TTF_Font* currentFont = Application::getInstance().font;
    TTF_SizeText(currentFont, title.c_str(), &titleWidth, nullptr);
    
    if (!thumbnail && thumbnailExists()) {
        thumbnail = loadThumbnail();
    }     
}

void SimpleMenuItem::deselect() {
    thumbnail = nullptr;
    scrollPixelPosition = 0;
}

SDL_Surface* SimpleMenuItem::getAssociatedBackground() const {
   if (background) {
        return background; 
    } else if (parentMenu) {
        return parentMenu->getBackground();
    }    
    return nullptr; // Default behavior if no parent menu is set
}

SDL_Surface* MenuItem::renderText(const std::string& text, SDL_Color color) {
    std::string titleFont = Configuration::getInstance().getValue("Menu.titleFont");
    int titleFontSize = Configuration::getInstance().getIntValue("Menu.titleFontSize");
    TTF_Font* font = TTF_OpenFont(titleFont.c_str(), titleFontSize);  // Adjust font path and size as necessary
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

void MenuItem::determineAndSetBackground(SDL_Surface* screen) {
    std::string backgroundPath = Configuration::getInstance().getThemePath() + "resources/" + this->getFolderName() + "/logo.png";
    std::cout << "background: " << backgroundPath << std::endl;
    if (background) {
        std::cout << "Background successfully set!" << std::endl;
        SDL_BlitSurface(background, NULL, screen, NULL);
    } else {
        std::cout << "Failed to set background!" << std::endl;
        setBackground(backgroundPath, screen);

    }
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

IntegerMenuItem::IntegerMenuItem(const std::string& name, 
                                 const std::string& value, 
                                 int min, 
                                 int max)
                    : SimpleMenuItem(name, "", value) {
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

