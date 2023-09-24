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
    Configuration& cfg = Configuration::getInstance();
    Theme& theme = Theme::getInstance();
    HelperUtils helper;
    SDL_Surface* thumbnail = nullptr;

    std::string currentBackgroundPath;

    static std::unordered_map<std::string, SDL_Surface*> thumbnailCache;
    static std::unordered_map<std::string, std::string> aliasMap;

    RenderComponent(); // Constructor is private now.
    ~RenderComponent(); // If needed

    // Common method to render text on screen
    void renderText(const std::string& text, int x, int y, SDL_Color color) {
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
    // Delete the copy constructor and copy assignment operator.
    RenderComponent(const RenderComponent&) = delete;
    RenderComponent& operator=(const RenderComponent&) = delete;

    void initialize(SDL_Surface* scr, TTF_Font* ft) {
        screen = scr;
        font = ft;
    }

    // Static method to access the singleton instance.
    static RenderComponent& getInstance() {
        static RenderComponent instance; // Guaranteed to be destroyed and instantiated on first use.
        return instance;
    }

    void drawSection(const std::string& name, const std::string& path, int numSystems);
    void drawFolder(const std::string& name, const std::string& path, int numRoms);
    void drawRomList(const std::vector<std::pair<std::string, std::string>>& romData, int currentRomIndex);
    void loadThumbnail(const std::string& romPath);
    void printFPS(int fps);
    void loadAliases();
    std::string getAlias(const std::string& title);
};
