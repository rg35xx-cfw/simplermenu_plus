#pragma once

#include <iostream>
#include <string>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include "Theme.h"


class RenderUtils {
private:
    Theme theme;
    TTF_Font* m_font;
    std::string m_fontPath;
    int m_fontSize;
    TTF_Font* generalFont;
    TTF_Font* textFont;

    static RenderUtils* instance;

public:
    static const int LEFT = 0;
    static const int CENTER = 1;
    static const int RIGHT = 2;

    // Default constructor
    RenderUtils(Theme& theme)
        : theme(theme) {
        
        instance = this;
        
        generalFont = TTF_OpenFont(theme.getValue(Configuration::THEME_FONT, true).c_str(),theme.getIntValue("GENERAL.font_size"));
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
    RenderUtils(Theme& theme, TTF_Font* font) : theme(theme), m_font(font) {}

    // Constructor to initialize with font path and size
    RenderUtils(Theme& theme, std::string fontPath, int fontSize) 
        : theme(theme), m_fontPath(fontPath), m_fontSize(fontSize) {
        m_font = TTF_OpenFont(fontPath.c_str(), fontSize);
        if (!m_font) {
            // Handle font loading error appropriately
            std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        }
    }

    static RenderUtils* getInstance(Theme& theme) {
        if (!instance) {
            instance = new RenderUtils(theme);
        }
        return instance;
    }

    static RenderUtils* getInstance() {
        if (!instance) {
            std::cerr << "RenderUtils instance still doesn't exist!" << std::endl;;
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

