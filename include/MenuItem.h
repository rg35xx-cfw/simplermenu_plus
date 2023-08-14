#pragma once
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_image.h>
#include <string>
#include <unordered_map>
#include <memory>
#include <filesystem>

#include "Menu.h"

class Menu;

class MenuItem {
protected:
    SDL_Surface* background = nullptr;
    Menu* parentMenu = nullptr;

public:
    virtual void executeAction() = 0;
    virtual void render(SDL_Surface* screen, TTF_Font* font, int x, int y, bool isSelected) = 0;
    virtual std::string getName() const = 0;

    virtual SDL_Surface* getAssociatedBackground() const = 0;

    virtual void determineAndSetBackground(SDL_Surface* screen) {
        // Default logic if any (can be empty for the base class)
    }

    static SDL_Surface* loadRomBackground() {
        std::string backgroundPath = Configuration::getInstance().getThemePath() + "resources/general/background.png";
        SDL_Surface* background = IMG_Load(backgroundPath.c_str());
        if (!background) {
            std::cerr << "Failed to load ROM background: " << IMG_GetError() << std::endl;
        }
        return background;
    }

    void setBackground(const std::string& backgroundPath, SDL_Surface* screen);
    
    SDL_Surface* getBackground() const {
        return background;
    }

    void setParentMenu(Menu* parent) {
        parentMenu = parent;
    }

    // Destructor to cleanup the background
    virtual ~MenuItem() {
        if (background) {
            SDL_FreeSurface(background);
        }
    }
};

class SimpleMenuItem : public MenuItem {
private:
    std::string title;
    std::string path;
    SDL_Surface* thumbnail = nullptr;
    static std::unordered_map<std::string, SDL_Surface*> thumbnailCache;
    std::string thumbnailPath;
    
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
        // Calculate thumbnail path here
        std::filesystem::path romPath(path);
        std::string romNameWithoutExtension = romPath.stem().string();
        std::string basePath = romPath.parent_path().string();
        thumbnailPath = basePath + "/media/images/" + romNameWithoutExtension + ".png";

    }

    ~SimpleMenuItem() {

    }

    const std::string& getPath() const {
        return path;
    }

    SDL_Surface* getAssociatedBackground() const override;

    void executeAction() override; 

    SDL_Surface* loadThumbnail();

    static void loadAliases();

    void render(SDL_Surface* screen, TTF_Font* font, int x, int y, bool isSelected) override;

    std::string getName() const override;

    bool thumbnailExists();

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

    SDL_Surface* getAssociatedBackground() const override;

    void executeAction() override;

    void render(SDL_Surface* screen, TTF_Font* font, int x, int y, bool isSelected) override;

    Menu* getSubMenu() const;

    std::string getName() const override;

    std::string getFolderName() const;

    void determineAndSetBackground(SDL_Surface* screen) override;

};
