#include <string>
#include <vector>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>
#include <filesystem>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_image.h>
#include <fstream>

#include "RenderComponent.h"

std::unordered_map<std::string, SDL_Surface*> RenderComponent::thumbnailCache;

std::unordered_map<std::string, std::string> RenderComponent::aliasMap;

RenderComponent::RenderComponent() {
    // Implementation
    loadAliases();
}

RenderComponent::~RenderComponent() {
    // Implementation
}

void RenderComponent::drawSection(const std::string& name, const std::string& path, int numSystems) {
    clearScreen();

    setBackground(path);

    if (theme.getValue("GENERAL.display_section_group_name") == "1") {
        SDL_Color white = {255, 255, 255};

        // Remove extension from section and transform to uppercase
        std::filesystem::path ss(name);
        std::string sectionName(ss.stem().string()); 
        transform(sectionName.begin(), sectionName.end(), sectionName.begin(), ::toupper);
    
        int sectionFontSize = 96;
        if(Configuration::getInstance().getIntValue(SettingId::SCREEN_WIDTH) == 320) {
            sectionFontSize = 48;
        }
    
        TTF_Font* titleFont = TTF_OpenFont(theme.getValue("GENERAL.font", true).c_str(), sectionFontSize);  // Adjust font path and size as necessary
        if (!titleFont) {
            // Handle error
            std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
            return;
        }       
        SDL_Surface* sectionNameSurface = TTF_RenderText_Blended(titleFont, sectionName.c_str(), {255,255,255});
        TTF_CloseFont(titleFont);   

        SDL_Rect dstRect;
        dstRect.x = (screen->w - sectionNameSurface->w) / 2;
        dstRect.y = (screen->h - sectionNameSurface->h) / 2;
        dstRect.w = sectionNameSurface->w;
        dstRect.h = sectionNameSurface->h;

        // Create a semi-transparent surface for the background
        SDL_Surface* transparentBg = SDL_CreateRGBSurface(0, cfg.getIntValue(SettingId::SCREEN_WIDTH), dstRect.h, 32, 0, 0, 0, 0);

        // Enable blending for the surface
        SDL_SetAlpha(transparentBg, SDL_SRCALPHA, 127);

        SDL_FillRect(transparentBg, NULL, SDL_MapRGBA(transparentBg->format, 0, 0, 0, 10)); // Fill with black color and 50% opacity

        SDL_Rect fadeRect = {0, Configuration::getInstance().getIntValue(SettingId::SCREEN_HEIGHT) / 2 - sectionNameSurface->h / 2,
                                Configuration::getInstance().getIntValue(SettingId::SCREEN_WIDTH),
                                Configuration::getInstance().getIntValue(SettingId::SCREEN_HEIGHT) / 2 + sectionNameSurface->h / 2};

        // Render the semi-transparent background
        SDL_BlitSurface(transparentBg, NULL, screen, &fadeRect);

        // Render the text on top of the semi-transparent background
        SDL_BlitSurface(sectionNameSurface, NULL, screen, &dstRect);

        SDL_FreeSurface(sectionNameSurface);
        SDL_FreeSurface(transparentBg); // Don't forget to free the surface when done
    }

    // Decide on the x, y positions, colors, and other styling details
    // Disable folder count displat for now, needs to be linked to a theme setting 
    // renderText("Folders: " + std::to_string(numSystems), 50, 150, {150, 150, 150}); // More gray for meta info, for example
}

void RenderComponent::drawFolder(const std::string& name, const std::string& path, int numRoms) {
    // clearScreen();

    std::string backgroundPath = Configuration::getInstance().getThemePath() + theme.getValue(name + ".logo");

    if(theme.getValue(name + ".logo") != "NOT FOUND") {
        // std::cout << "background path: " << backgroundPath << std::endl;
        setBackground(backgroundPath);

    } else {
        clearScreen();
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
    //clearScreen();
    // int startY = 50;
    // int stepY = 30; // spacing between ROM names

    std::string backgroundPath = Configuration::getInstance().getValue(SettingId::THEME_PATH) + 
                                 std::to_string(Configuration::getInstance().getIntValue(SettingId::SCREEN_WIDTH)) + "x" +
                                 std::to_string(Configuration::getInstance().getIntValue(SettingId::SCREEN_HEIGHT)) + "/" +
                                 Configuration::getInstance().getValue(SettingId::THEME_NAME) + "/" +
                                 theme.getValue("DEFAULT.background");

    setBackground(backgroundPath);

    // Set rom list starting position and item separation
    int startX = theme.getIntValue("GENERAL.game_list_x");
    int startY = theme.getIntValue("GENERAL.game_list_y");
    int stepY = theme.getIntValue("GENERAL.items_separation");

    // TODO: add page logic
    for (int i = 0; i < romData.size(); i++) {
        SDL_Color color = (i == currentRomIndex) ? theme.getColor("DEFAULT.selected_item_font_color"):theme.getColor("DEFAULT.items_font_color");
        std::string alias = getAlias(romData[i].first);
        renderText(alias, startX, startY, color); // Render ROM name
        // Debug path info, disabled by default
        //renderText(romData[i].second, 250, startY, color); // Render ROM path
        startY += stepY;
    }

    loadThumbnail(romData[currentRomIndex].second);

    Uint16 x = theme.getIntValue("GENERAL.art_x"); 
    Uint16 y = theme.getIntValue("GENERAL.art_y"); 
    Uint16 w = theme.getIntValue("GENERAL.art_max_w"); 
    Uint16 h = theme.getIntValue("GENERAL.art_max_h"); 
    SDL_Rect destRect = {x, y, w, h};
    SDL_BlitSurface(thumbnail, nullptr, screen, &destRect);

}

void RenderComponent::loadThumbnail(const std::string& romPath) {

    std::filesystem::path path(romPath);
    std::string romNameWithoutExtension = path.stem().string();
    std::string basePath = path.parent_path().string();
    std::string thumbnailPath = basePath + "/media/images/" + romNameWithoutExtension + ".png";

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
}

void RenderComponent::printFPS(int fps) {
    // Display FPS page number / total_pages at the bottom
    std::string fpsText = "FPS: " + std::to_string(fps);

    SDL_Surface* rawTextSurface = TTF_RenderText_Blended(font, fpsText.c_str(), {255,255,0});
    if (!rawTextSurface) {
        return;
    }

    SDL_Surface* textSurface = SDL_DisplayFormatAlpha(rawTextSurface);
    SDL_FreeSurface(rawTextSurface);

    if(!textSurface) {
        return;
    }

    SDL_Rect destRect = {10, 10, 0, 0};  // Positon for page counter
    SDL_BlitSurface(textSurface, NULL, screen, &destRect);

    SDL_FreeSurface(textSurface);
}

void RenderComponent::loadAliases() {
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

std::string RenderComponent::getAlias(const std::string& title) {
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
    
    return displayTitle;
}