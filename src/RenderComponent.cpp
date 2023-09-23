#include <string>
#include <vector>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>
#include <filesystem>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_image.h>

#include "RenderComponent.h"


std::unordered_map<std::string, SDL_Surface*> RenderComponent::thumbnailCache;

RenderComponent::RenderComponent() {
    // Implementation
}

RenderComponent::~RenderComponent() {
    // Implementation
}

void RenderComponent::drawSection(const std::string& name, const std::string& path, int numSystems) {
    clearScreen();

    setBackground(path);

    // Decide on the x, y positions, colors, and other styling details
    renderText(name, 50, 50, {255, 255, 255}); // White color, for example
    renderText("Folders: " + std::to_string(numSystems), 50, 150, {150, 150, 150}); // More gray for meta info, for example
}

void RenderComponent::drawSystem(const std::string& name, const std::string& path, int numRoms) {
    clearScreen();

    std::string backgroundPath = Configuration::getInstance().getThemePath() + theme.getValue(name + ".logo");

    if(theme.getValue(name + ".logo") != "NOT FOUND") {
        // std::cout << "background path: " << backgroundPath << std::endl;
        setBackground(backgroundPath);
    }

    // As before, determine x, y positions and styles
    renderText(name, 50, 50, {255, 255, 255}); 
    renderText(path, 50, 100, {200, 200, 200}); 

    if(theme.getIntValue("GENERAL.display_game_count") == 1 ) {
        int x = theme.getIntValue("GENERAL.game_count_x");
        int y = theme.getIntValue("GENERAL.game_count_y");
        SDL_Color color = theme.getColor("GENERAL.game_count_font_color");
        renderText(std::to_string(numRoms) + " GAMES", x, y, color);
    }

}

void RenderComponent::drawRomList(const std::vector<std::pair<std::string, std::string>>& romData, int currentRomIndex) {
    clearScreen();
    int startY = 50;
    int stepY = 30; // spacing between ROM names

    std::string backgroundPath = Configuration::getInstance().getValue(SettingId::THEME_PATH) + 
                                 std::to_string(Configuration::getInstance().getIntValue(SettingId::SCREEN_WIDTH)) + "x" +
                                 std::to_string(Configuration::getInstance().getIntValue(SettingId::SCREEN_HEIGHT)) + "/" +
                                 Configuration::getInstance().getValue(SettingId::THEME_NAME) + "/" +
                                 theme.getValue("DEFAULT.background");

    setBackground(backgroundPath);

    for (int i = 0; i < romData.size(); i++) {
        SDL_Color color = (i == currentRomIndex) ? SDL_Color{255, 0, 0} : SDL_Color{255, 255, 255}; // Red for selected, White for others
        renderText(romData[i].first, 50, startY, color); // Render ROM name
        renderText(romData[i].second, 250, startY, color); // Render ROM path
        loadThumbnail(romData[i].second);
        startY += stepY;
    }
}

void RenderComponent::loadThumbnail(const std::string& romPath) {

    std::string thumbnailPath = "/userdata/roms/neogeo/media/images/viewpoin.png";
    SDL_Surface* thumbnail = nullptr;
    SDL_Surface* tmpThumbnail = nullptr;

    // If thumbnail is already in cache, set it and return
    if (thumbnailCache.find(thumbnailPath) != thumbnailCache.end()) {
        thumbnail = thumbnailCache[thumbnailPath];
        return;
    }

    // If thumbnail exists, proceed with loading and caching
    if (std::filesystem::exists(thumbnailPath)) { 
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
    Uint16 x = 100;//theme.getIntValue("GENERAL.art_x"); 
    Uint16 y = 40;//theme.getIntValue("GENERAL.art_y"); 
    Uint16 w = 320;//theme.getIntValue("GENERAL.art_max_w"); 
    Uint16 h = 240;//theme.getIntValue("GENERAL.art_max_h"); 
    SDL_Rect destRect = {x, y, w, h};
    SDL_BlitSurface(thumbnail, nullptr, screen, &destRect);
}

