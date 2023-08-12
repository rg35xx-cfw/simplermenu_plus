#include "MenuItem.h"
#include "Menu.h"
#include "Application.h"

#include <iostream>
#include <filesystem>
#include <memory>
#include <fstream>

void SimpleMenuItem::executeAction() {
    std::cout << "executeAction" << std::endl;
    // Execute the action associated with this menu item
    // This can be a callback or some other function
    if (title == "Back") {
    // This is a simplistic way to handle it. 
    // A better way would be to use callbacks or command patterns.
        Application::getInstance().showMainMenu();
    } else if (title == "Return to Main Menu") {
        Application::getInstance().showMainMenu();
    } else if (!path.empty()) {
        std::cout << "Launching rom: " << path << std::endl;
    }
}

SDL_Surface* SimpleMenuItem::loadThumbnail(const std::string& title, const std::string& romPath) {
    if (thumbnailExists(romPath)) {
        std::filesystem::path path(romPath);
        std::string romNameWithoutExtension = path.stem().string();
        std::string basePath = path.parent_path().string();
        std::string imagesPath = Configuration::getInstance().getValue("Menu.imagesPath");
        std::string thumbnailPath = basePath + imagesPath + romNameWithoutExtension + ".png";

        SDL_Surface* thumbnail = Application::getInstance().getThumbnailCache().get(thumbnailPath);

        if (!thumbnail && thumbnailExists(romPath)) {
            SDL_Surface* original = IMG_Load(thumbnailPath.c_str());
            int thumbnailWidth = Configuration::getInstance().getIntValue("Menu.thumbnailWidth");
            int thumbnailHeight = Configuration::getInstance().getIntValue("Menu.thumbnailHeight");

            if (original->w > thumbnailWidth || original->h > thumbnailHeight) {
                double scaleX = (double)thumbnailWidth / original->w;
                double scaleY = (double)thumbnailHeight / original->h;
                double scale = std::min(scaleX,scaleY);

                thumbnail = zoomSurface(original, scale, scale, SMOOTHING_ON);
                SDL_FreeSurface(original);
            } else {
                thumbnail = original;
            }
            Application::getInstance().getThumbnailCache().set(thumbnailPath, thumbnail);
            if (!thumbnail) {
                std::cerr << "Error loading thumbnail: " << IMG_GetError() << std::endl;
            }
        }
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

void SimpleMenuItem::render(SDL_Surface* screen, TTF_Font* font, int x, int y, bool isSelected) {
    // Set the colors, yellow for selected text, white for non-selected text
    SDL_Color textColor = isSelected ? SDL_Color{255,255,0}: SDL_Color{255, 255, 255}; 

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
        if (scrollPixelPosition < titleWidth - 220) { 
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

    SDL_Surface* thumbnail = loadThumbnail(title, path);
    if (isSelected && thumbnail) {
        SDL_Rect destRect = {static_cast<Sint16>(screen->w / 2 - 20), 100, 0, 0};
        SDL_BlitSurface(thumbnail, nullptr, screen, &destRect);  
    }

    SDL_FreeSurface(textSurface);
}

std::string SimpleMenuItem::getName() const {
    return title;
}

bool SimpleMenuItem::thumbnailExists(const std::string& romPath) {
    std::filesystem::path path(romPath);
    std::string romNameWithoutExtension = path.stem().string();
    std::string basePath = path.parent_path().string();
    std::string thumbnailPath = basePath + "/media/images/" + romNameWithoutExtension + ".png";
    return std::filesystem::exists(thumbnailPath);
}

void SimpleMenuItem::select() {
    scrollPixelPosition = 0;
    selectTime = SDL_GetTicks();
    
    // Measure the width of the title
    TTF_Font* currentFont = Application::getInstance().font;
    TTF_SizeText(currentFont, title.c_str(), &titleWidth, nullptr);
    
    if (!thumbnail && thumbnailExists(path)) {
        thumbnail = loadThumbnail(title, path);
    }     
}

void SimpleMenuItem::deselect() {
    thumbnail = nullptr;
    scrollPixelPosition = 0;
}

void SubMenuMenuItem::executeAction() {
    // This will typically switch the current menu to the submenu
    // For this, you might need to communicate with the Application or State class
    // Alternatively, you can just display a message or perform some other action
    std::cout << "ExecuteAction" << std::endl;
}

void SubMenuMenuItem::render(SDL_Surface* screen, TTF_Font* font, int x, int y, bool isSelected) {
    if (!Application::getInstance().getBackground()) {
        // If there's no background, render the folder name centered
        SDL_Color white = {255, 255, 255};
        SDL_Surface* folderNameSurface = Application::getInstance().renderText(title, white);
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
}

Menu* SubMenuMenuItem::getSubMenu() const {
    return submenu.get();
}

std::string SubMenuMenuItem::getName() const {
    return title + " >";
}

std::string SubMenuMenuItem::getFolderName() const {
    return title;
}

