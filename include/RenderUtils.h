#include <iostream>
#include <string>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>


class RenderUtils {
public:
    static const int LEFT = 0;
    static const int CENTER = 1;
    static const int RIGHT = 2;

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

    void renderText(SDL_Surface* screen, const std::string& text, int x, int y, int w, int h, SDL_Color color, int align = LEFT) {
        SDL_Surface* textSurface = TTF_RenderText_Blended(m_font, text.c_str(), color);
        
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
        if (m_font) {
            TTF_CloseFont(m_font);
        }
        m_font = TTF_OpenFont(fontPath.c_str(), size);
        if (!m_font) {
            // Handle font loading error appropriately
            std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        }
    }

private:
    TTF_Font* m_font;
    std::string m_fontPath;
    int m_fontSize;
};

