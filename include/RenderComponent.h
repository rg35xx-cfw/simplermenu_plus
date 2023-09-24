#pragma once

#include <string>
#include <vector>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>
#include <iostream>
#include "Configuration.h"
#include "Theme.h"
#include "HelperUtils.h"

class RenderComponent {
private:
    SDL_Surface* screen;
    TTF_Font* font;
    SDL_Surface* background;
    Configuration cfg;
    Theme theme;
    HelperUtils helper;
    SDL_Surface* thumbnail = nullptr;

    int screenHeight;
    int screenWidth;

    std::string currentBackgroundPath;

    static std::unordered_map<std::string, SDL_Surface*> thumbnailCache;
    static std::unordered_map<std::string, std::string> aliasMap;

    // Common method to render text on screen
    void renderText(const std::string& text, Sint16 x, Sint16 y, SDL_Color color) {
        SDL_Surface* rawTextSurface = TTF_RenderText_Blended(font, text.c_str(), color);
        if (!rawTextSurface) {
            // Handle the error, e.g., print an error message
            return;
        }

        // Convert the surface to the display format while preserving alpha
        SDL_Surface* textSurface = SDL_DisplayFormatAlpha(rawTextSurface);
        SDL_FreeSurface(rawTextSurface);  // Free the original surface

        if (!textSurface) {
            // Handle the error, e.g., print an error message
            return;
        }

        SDL_Rect position = {x, y, 0, 0};  // Assuming width and height are determined by the textSurface
        SDL_BlitSurface(textSurface, NULL, screen, &position);

        SDL_FreeSurface(textSurface);  // Free the converted surface
    }

    void setBackground(const std::string& backgroundPath) {
        SDL_Surface* loadedSurface = IMG_Load(backgroundPath.c_str());
        if (!loadedSurface) {
            std::cerr << "Failed to load background: " << IMG_GetError() << std::endl;
            return;
        }

        // Free the old background surface if it exists
        if (background) {
            SDL_FreeSurface(background);
        }

        background = SDL_DisplayFormat(loadedSurface);
        SDL_FreeSurface(loadedSurface);

        if (!background) {
            std::cerr << "Failed to convert background format: " << IMG_GetError() << std::endl;
            return;
        }

        SDL_BlitSurface(background, NULL, screen, NULL);
    }

    void clearScreen() {
        SDL_FillRect(screen, nullptr, SDL_MapRGB(screen->format, 0, 0, 0));  // Filling with black
    }

public:

    RenderComponent(Configuration& cfg);
    ~RenderComponent(); // If needed

    void initialize() {

        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            // Handle error
            exit(1);
        }

        screen = SDL_SetVideoMode(
            cfg.getInt("MENU.screenWidth"),
            cfg.getInt("MENU.screenHeight"),
            cfg.getInt("MENU.screenDepth"),
            SDL_HWSURFACE | SDL_DOUBLEBUF);

        if (!screen) {
            std::cerr << "Unable to set video mode: " << SDL_GetError() << std::endl;
            SDL_Quit();
            exit(1);
        }

        if (TTF_Init() == -1) {
            std::cerr << "Unable to initialize TTF: " << TTF_GetError() << std::endl;
            SDL_Quit();
            exit(1);
        }

        font = TTF_OpenFont(
        theme.getValue("GENERAL.font", true).c_str(),
        theme.getIntValue("GENERAL.font_size"));
        TTF_SetFontHinting(font, TTF_HINTING_NORMAL);  // or TTF_HINTING_LIGHT, TTF_HINTING_MONO, TTF_HINTING_NONE
        TTF_SetFontKerning(font, 1); // 1 to enable, 0 to disable

        // Enable keyboard repeat (only for keys, not for buttons)
        SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
            
    }

    void drawSection(const std::string& name, int numSystems);
    void drawFolder(const std::string& name, const std::string& path, int numRoms);
    void drawRomList(const std::vector<std::pair<std::string, std::string>>& romData, int currentRomIndex);
    void loadThumbnail(const std::string& romPath);
    void printFPS(int fps);
    void loadAliases();
    std::string getAlias(const std::string& title);
    
    void update();
};
