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

RenderComponent::RenderComponent(Configuration& cfg) 
    : cfg(cfg), 
      theme(cfg.get("MENU.themeName"), cfg.getInt("MENU.screenWidth"), cfg.getInt("MENU.screenHeight")) {

    screenHeight = cfg.getInt("MENU.screenHeight");
    screenWidth = cfg.getInt("MENU.screenWidth");

    lastSection = "";
    lastFolder = "";
    lastRom = -1;

    // Implementation
    loadAliases();
}

RenderComponent::~RenderComponent() {
    if(background) {
        SDL_FreeSurface(background);
    }
    if(thumbnail) {
        SDL_FreeSurface(thumbnail);
    }
    if(tmpThumbnail) {
        SDL_FreeSurface(tmpThumbnail);
    }
    if(font) {
        TTF_CloseFont(font);
    }
    if(screen) {
        SDL_FreeSurface(screen);
    }
    // Implementation
}

void RenderComponent::drawSection(const std::string& name, int numSystems) {

    std::string backgroundPath = 
                cfg.get("MENU.themePath") + 
                std::to_string(cfg.getInt("MENU.screenWidth")) + "x" +
                std::to_string(cfg.getInt("MENU.screenHeight")) + "/" +
                cfg.get("MENU.themeName") + "/" +
                theme.getValue("GENERAL.section_groups_folder") +
                helper.getFilenameWithoutExtension(name) + ".png";

    if(background == nullptr || lastSection != name) {
    	setBackground(backgroundPath);
        lastSection = name;
    }
    SDL_BlitSurface(background, NULL, screen, NULL);


    if (theme.getValue("GENERAL.display_section_group_name") == "1") {
        SDL_Color white = {255, 255, 255};

        // Remove extension from section and transform to uppercase
        std::filesystem::path ss(name);
        std::string sectionName(ss.stem().string()); 
        transform(sectionName.begin(), sectionName.end(), sectionName.begin(), ::toupper);
    
        int sectionFontSize = 96;
        if(screenWidth == 320) {
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

        // Add section title with translucent background
        SDL_Rect dstRect;
        dstRect.x = (screen->w - sectionNameSurface->w) / 2;
        dstRect.y = (screen->h - sectionNameSurface->h) / 2;
        dstRect.w = sectionNameSurface->w;
        dstRect.h = sectionNameSurface->h;

        // Create a semi-transparent surface for the background
        SDL_Surface* rawTransparentBg = SDL_CreateRGBSurface(0, screenWidth, dstRect.h, 32, 0, 0, 0, 0);

        // Enable blending for the surface
        SDL_SetAlpha(rawTransparentBg, SDL_SRCALPHA, 127);

        SDL_FillRect(rawTransparentBg, NULL, SDL_MapRGBA(rawTransparentBg->format, 0, 0, 0, 10)); // Fill with black color and 50% opacity

        SDL_Surface* transparentBg = SDL_DisplayFormatAlpha(rawTransparentBg);

        SDL_Rect fadeRect = {0, screenHeight / 2 - sectionNameSurface->h / 2,
                            screenWidth,
                            screenHeight / 2 + sectionNameSurface->h / 2};

        // Render the semi-transparent background
        SDL_BlitSurface(transparentBg, NULL, screen, &fadeRect);

        SDL_Surface* convertedFolderNameSurface = SDL_DisplayFormatAlpha(sectionNameSurface);

        // Render the text on top of the semi-transparent background
        SDL_BlitSurface(convertedFolderNameSurface, NULL, screen, &dstRect);

        // Free surfaces
        SDL_FreeSurface(sectionNameSurface);
        SDL_FreeSurface(rawTransparentBg);
        SDL_FreeSurface(transparentBg);
        SDL_FreeSurface(convertedFolderNameSurface);
    }

    // Decide on the x, y positions, colors, and other styling details
    // Disable folder count displat for now, needs to be linked to a theme setting 
    // renderText("Folders: " + std::to_string(numSystems), 50, 150, {150, 150, 150}); // More gray for meta info, for example
}

void RenderComponent::drawFolder(const std::string& name, const std::string& path, int numRoms) {
    std::string backgroundPath = cfg.getThemePath() + theme.getValue(name + ".logo");

    if(theme.getValue(name + ".logo") != "NOT FOUND") {
	    if(background == nullptr || lastFolder != name) {
       	    setBackground(backgroundPath);
            lastFolder = name;
    	}
    	SDL_BlitSurface(background, NULL, screen, NULL);
    } else {
        clearScreen();
        renderText(name, cfg.getInt("MENU.screenWidth") / 2 , cfg.getInt("MENU.screenHeight") / 2 , {255, 255, 255}, 1); 
    }

    // As before, determine x, y positions and styles
    //renderText(name, 50, 50, {255, 255, 255}); 
    //renderText(path, 50, 100, {200, 200, 200}); 

    if(theme.getIntValue("GENERAL.display_game_count") == 1 ) {
        int x = theme.getIntValue("GENERAL.game_count_x");
        int y = theme.getIntValue("GENERAL.game_count_y");
        SDL_Color color = theme.getColor("GENERAL.game_count_font_color");
        renderText(std::to_string(numRoms) + " GAMES", x, y, color, theme.getIntValue("GENERAL.game_count_alignment"));
    }

}

void RenderComponent::drawRomList(const std::string& folderName, const std::vector<std::pair<std::string, std::string>>& romData, int currentRomIndex) {

    std::string backgroundPath = cfg.get("MENU.themePath") + 
                                 std::to_string(screenWidth) + "x" +
                                 std::to_string(screenHeight) + "/" +
                                 cfg.get("MENU.themeName") + "/" +
                                 theme.getValue("DEFAULT.background");

	if (background == nullptr || lastRom == -1) {
        setBackground(backgroundPath);
        lastRom = currentRomIndex;
    }
    SDL_BlitSurface(background, NULL, screen, NULL);

    // Set rom list starting position and item separation
    int startX = theme.getIntValue("GENERAL.game_list_x");
    int startY = theme.getIntValue("GENERAL.game_list_y");
    int stepY = theme.getIntValue("GENERAL.items_separation");

    int itemsPerPage = theme.getIntValue("GENERAL.items");

    // Calculate number of pages FIXME: move that to the constructor
    int total_pages = (romData.size() + itemsPerPage - 1)/ itemsPerPage;

    int currentPage = currentRomIndex / itemsPerPage;
    int startIndex = currentPage * itemsPerPage;
    int endIndex = std::min<int>(startIndex + itemsPerPage, romData.size());

    // for (int i = 0; i < theme.getIntValue("GENERAL.items"); i++) {
    for (int i = startIndex; i < endIndex; i++) {
        SDL_Color color = (i == currentRomIndex) ? theme.getColor("DEFAULT.selected_item_font_color"):theme.getColor("DEFAULT.items_font_color");
        std::string alias = getAlias(romData[i].first);

        // Determine text width
        SDL_Surface* textSurface = TTF_RenderText_Blended(font, alias.c_str(), color);
        int titleWidth = textSurface->w;

        // TODO replace clipWidth the correct width based on theme.ini settings
        int clipWidth = theme.getIntValue("GENERAL.game_list_w");

        // Create the scrolling view for titles that are too wide
        if(i == currentRomIndex) {
            if (i == currentRomIndex && SDL_GetTicks() - selectTime > SCROLL_TIMEOUT) {
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
            SDL_Rect destRect = {static_cast<Sint16>(startX - scrollPixelPosition), startY, 0, 0};  // Adjust x position by scrollPixelPosition
            SDL_Rect clipRect = {startX, startY, clipWidth, static_cast<Uint16>(textSurface->h)}; // Ensure text doesn't spill over the intended area

            SDL_SetClipRect(screen, &clipRect);
            SDL_BlitSurface(textSurface, nullptr, screen, &destRect);
        } else {
            SDL_Rect clipRect = {startX, startY, clipWidth, static_cast<Uint16>(textSurface->h)}; // Ensure text doesn't spill over the intended area

            SDL_SetClipRect(screen, &clipRect);
            SDL_BlitSurface(textSurface, nullptr, screen, &clipRect);
        }

        SDL_SetClipRect(screen, NULL);  // Reset the clip rect
        SDL_FreeSurface(textSurface);

        if (i == currentRomIndex) {
            // Add Rom title 
            int x = theme.getIntValue("GENERAL.art_x") + theme.getIntValue("GENERAL.art_max_w")/2;
            int y = theme.getIntValue("GENERAL.art_y") +
                    theme.getIntValue("GENERAL.art_max_h") +
                    theme.getIntValue("GENERAL.art_text_distance_from_picture") +
                    theme.getIntValue("GENERAL.art_text_line_separation");
        
            if(alias.find("/") != std::string::npos) {
                size_t position = alias.find("/");
                std::string firstLine = alias.substr(0, position - 1);
                std::string secondLine = alias.substr(position + 2);
                int separation = theme.getIntValue("GENERAL.art_text_line_separation") / 2;

                renderText(firstLine, x, y - separation, {255, 255, 255},1);
                renderText(secondLine, x, y + separation, {255, 255, 255},1);
            } else {
                renderText(alias, x, y, {255, 255, 255},1);
            }
        }



        // Display pagination page number / total_pages at the bottom
        std::string pageInfo = std::to_string(currentPage + 1) + " / " + std::to_string(total_pages);
        int x = theme.getIntValue("GENERAL.text2_x");
        int y = theme.getIntValue("GENERAL.text2_y");

        renderText(pageInfo, x, y, {255, 255, 255}, theme.getIntValue("GENERAL.text2_alignment"));

        startY += stepY;
    }

    // Load Thumbnail
    if(thumbnail == nullptr || lastRom != currentRomIndex) {
        loadThumbnail(romData[currentRomIndex].second);
        lastRom = currentRomIndex;
    }
    Sint16 x = theme.getIntValue("GENERAL.art_x"); 
    Sint16 y = theme.getIntValue("GENERAL.art_y"); 
    Uint16 w = theme.getIntValue("GENERAL.art_max_w"); 
    Uint16 h = theme.getIntValue("GENERAL.art_max_h"); 
    SDL_Rect destRect = {x, y, w, h};
    SDL_BlitSurface(thumbnail, nullptr, screen, &destRect);

    // Add Folder Title
    renderText(folderName, theme.getIntValue("GENERAL.text1_x"), theme.getIntValue("GENERAL.text1_y"), {255, 255, 255}, theme.getIntValue("GENERAL.text1_alignment")); 
}

void RenderComponent::loadThumbnail(const std::string& romPath) {
    //std::cout << "loadThumbnail called for " << romPath << std::endl;

    std::filesystem::path path(romPath);
    std::string romNameWithoutExtension = path.stem().string();
    std::string basePath = path.parent_path().string();
    std::string thumbnailPath = basePath + "/media/images/" + romNameWithoutExtension + ".png";

    // If thumbnail is already in cache, set it and return
    if (thumbnailCache.find(thumbnailPath) != thumbnailCache.end()) {
        thumbnail = thumbnailCache[thumbnailPath];
        //std::cout << "loadThumbnail found in cache, returning" << std::endl;
        return;
    }

    // If the thumbnail doesn't exist, simply return
    if (!std::filesystem::exists(thumbnailPath)) {
        return;
    }

    tmpThumbnail = IMG_Load(thumbnailPath.c_str());

    int thumbnailWidth = theme.getIntValue("GENERAL.art_max_w");
    int thumbnailHeight = theme.getIntValue("GENERAL.art_max_h");

    // Check if the thumbnail needs to be resized
    if (tmpThumbnail->w != thumbnailWidth || tmpThumbnail->h != thumbnailHeight) {
        double scaleX = (double)thumbnailWidth / tmpThumbnail->w;
        double scaleY = (double)thumbnailHeight / tmpThumbnail->h;
        double scale = std::min(scaleX, scaleY);

        SDL_Surface* resizedThumbnail = zoomSurface(tmpThumbnail, scale, scale, SMOOTHING_ON);

        if (tmpThumbnail) {
            SDL_FreeSurface(tmpThumbnail);
        }

        SDL_Surface* loadedSurface = resizedThumbnail;
        if (loadedSurface) {
            tmpThumbnail = SDL_DisplayFormat(loadedSurface);
            SDL_FreeSurface(loadedSurface);
        } else {
            std::cerr << "Failed to load thumbnail: " << IMG_GetError() << std::endl;
        }

        // // Free the original loaded thumbnail as it's no longer needed
        // SDL_FreeSurface(tmpThumbnail);
        // tmpThumbnail = resizedThumbnail;
    }

    if (thumbnailCache.find(thumbnailPath) != thumbnailCache.end()) {
        thumbnailCache.erase(thumbnailPath);
    }

    // Cache and set the thumbnail
    thumbnailCache[thumbnailPath] = tmpThumbnail;
    thumbnail = tmpThumbnail;
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
    std::ifstream infile(cfg.get("MENU.aliasPath"));
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

void RenderComponent::update() {
    if (SDL_Flip(screen) == -1) {
        std::cerr << "SDL_Flip failed: " << SDL_GetError() << std::endl;
        return;  // or handle the error as appropriate
    }
}
