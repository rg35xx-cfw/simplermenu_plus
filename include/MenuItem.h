#pragma once
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_image.h>
#include <string>
#include <unordered_map>
#include <memory>

#include "Menu.h"

class Menu;

class MenuItem {
public:
    virtual void executeAction() = 0;
    virtual void render(SDL_Surface* screen, TTF_Font* font, int x, int y, bool isSelected) = 0;
    virtual std::string getName() const = 0;
};

class SimpleMenuItem : public MenuItem {
private:
    std::string title;
    std::string path;
    SDL_Surface* thumbnail = nullptr;
    static std::unordered_map<std::string, std::string> aliasMap;

    // Text scroll
    int scrollPixelPosition = 0;
    Uint32 scrollEndTime = 0;
    Uint32 selectTime = 0;
    int titleWidth = 0;
    static const Uint32 SCROLL_TIMEOUT = 2000;
    static const Uint32 SCROLL_SPEED = 600;
    static const Uint32 END_SCROLL_PAUSE = 3000;
    int thumbnailOffset_x;
    int thumbnailOffset_y;

    // Potentially other attributes like action or callback

public:
    SimpleMenuItem(const std::string& title, const std::string& path = "") : title(title), path(path) {

    }

    ~SimpleMenuItem() {

    }

    void executeAction() override; 

    SDL_Surface* loadThumbnail(const std::string& title, const std::string& romPath);

    static void loadAliases();

    void render(SDL_Surface* screen, TTF_Font* font, int x, int y, bool isSelected) override;

    std::string getName() const override;

    bool thumbnailExists(const std::string& romPath);

    void select();

    void deselect();
};

class SubMenuMenuItem : public MenuItem {
private:
    std::string title;
    std::unique_ptr<Menu> submenu;

public:
    SubMenuMenuItem(const std::string& title, std::unique_ptr<Menu> submenu)
        : title(title), submenu(std::move(submenu)) {}

    void executeAction() override;

    void render(SDL_Surface* screen, TTF_Font* font, int x, int y, bool isSelected) override;

    Menu* getSubMenu() const;

    std::string getName() const override;

    std::string getFolderName() const;

};