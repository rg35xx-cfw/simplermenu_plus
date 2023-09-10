#pragma once

#include <iostream>
#include <string>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include "Theme.h"


class RenderUtils {
private:
    TTF_Font* m_font;
    std::string m_fontPath;
    int m_fontSize;
    TTF_Font* generalFont;
    TTF_Font* textFont;
    Theme& theme = Theme::getInstance();
    

    static RenderUtils* instance;

public:
    static const int LEFT = 0;
    static const int CENTER = 1;
    static const int RIGHT = 2;

    // Default constructor
    RenderUtils() {
        instance = this;
        
        generalFont = TTF_OpenFont(theme.getValue("GENERAL.font", true).c_str(),theme.getIntValue("GENERAL.font_size"));
        if (!generalFont) {
            // Handle font loading error appropriately
            std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        }

        textFont = TTF_OpenFont(theme.getValue("GENERAL.textX_font", true).c_str(),theme.getIntValue("GENERAL.font_size"));
        if (!textFont) {
            // Handle font loading error appropriately
            std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        }

        m_font = generalFont;
    }

    // Constructor to initialize with a font
    RenderUtils(TTF_Font* font) : m_font(font) {}

    // Constructor to initialize with font path and size
    RenderUtils(std::string fontPath, int fontSize) : m_fontPath(fontPath), m_fontSize(fontSize) {
        m_font = TTF_OpenFont(fontPath.c_str(), fontSize);
        if (!m_font) {
            // Handle font loading error appropriately
            std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        }
    }

    static RenderUtils* getInstance() {
        if (!instance) {
            instance = new RenderUtils();
        }
        return instance;
    }

    ~RenderUtils() {
        if(m_font) {
            TTF_CloseFont(m_font);
        }
    }

    void renderText(SDL_Surface* screen, const std::string& font, const std::string& text, int x, int y, int w, int h, SDL_Color color, int align = LEFT) {
        SDL_Surface* textSurface = TTF_RenderText_Blended(font == "generalFont" ? generalFont : textFont, text.c_str(), color);
        
        SDL_Rect destRect;
        switch(align) {
            case CENTER:
                destRect.x = x + (w - textSurface->w) / 2;
                destRect.y = y + (h - textSurface->h) / 2;
                break;
            case RIGHT:
                destRect.x = x + w - textSurface->w;
                destRect.y = y + (h - textSurface->h) / 2;
                break;
            case LEFT:
            default:
                destRect.x = x;
                destRect.y = y - textSurface->h / 2;
                break;
        }
        
        SDL_BlitSurface(textSurface, NULL, screen, &destRect);
        SDL_FreeSurface(textSurface);
    }

    // Method to set a new font and size
    void setFont(const std::string& fontPath, int size) {
        // if (m_font) {
        //     TTF_CloseFont(m_font);
        // }
        m_font = TTF_OpenFont(fontPath.c_str(), size);
        if (!m_font) {
            // Handle font loading error appropriately
            std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        }
    }
};

