#pragma once

#include <string>
#include <vector>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>
#include <iostream>
#include "Configuration.h"
#include "Theme.h"

class RenderComponent {
private:
    SDL_Surface* screen;
    TTF_Font* font;
    SDL_Surface* background;
    Configuration& cfg = Configuration::getInstance();
    Theme& theme = Theme::getInstance();

    static std::unordered_map<std::string, SDL_Surface*> thumbnailCache;

    RenderComponent(); // Constructor is private now.
    ~RenderComponent(); // If needed

    // Common method to render text on screen
    void renderText(const std::string& text, int x, int y, SDL_Color color) {
        SDL_Surface* textSurface = TTF_RenderText_Blended(font, text.c_str(), color);
        SDL_Rect position = {x, y, 0, 0};  // Assuming width and height are determined by the textSurface
        SDL_BlitSurface(textSurface, NULL, screen, &position);
        SDL_FreeSurface(textSurface);
    }

    void setBackground(const std::string& backgroundPath) {
        SDL_Surface* loadedSurface = IMG_Load(backgroundPath.c_str());
        if(loadedSurface) {
            background = SDL_DisplayFormat(loadedSurface);
            SDL_FreeSurface(loadedSurface);
            SDL_BlitSurface(background, NULL, screen, NULL);
        } else {
            std::cerr << "Failed to load background: " << IMG_GetError() << std::endl;
        }
        if (!background) {
            std::cerr << "Failed to load background: " << IMG_GetError() << std::endl;
        }
        SDL_FreeSurface(background);
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
    void drawSystem(const std::string& name, const std::string& path, int numRoms);
    void drawRomList(const std::vector<std::pair<std::string, std::string>>& romData, int currentRomIndex);
    void loadThumbnail(const std::string& romPath);

    void printFPS(int fps) {
        // Display FPS page number / total_pages at the bottom
        std::string fpsText = "FPS: " + std::to_string(fps);
        SDL_Surface* textSurface = TTF_RenderText_Blended(font, fpsText.c_str(), {255,255,0});
        SDL_Rect destRect = {10, 10, 0, 0};  // Positon for page counter
        SDL_BlitSurface(textSurface, NULL, screen, &destRect);
    }

};
