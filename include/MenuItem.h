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

    std::string title;
    std::string value;

public:

    MenuItem(const std::string& title, const std::string& value = "") : title(title), value(value) {}

    virtual void navigateLeft() {}
    virtual void navigateRight() {}

    virtual void executeAction() = 0;
    virtual void render(SDL_Surface* screen, TTF_Font* font, int x, int y, bool isSelected) = 0;

    virtual void renderTitle() const {};
    virtual void renderValue() const {};

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

    // FIXME needs to be part of a different helper/utils class
    SDL_Surface* renderText(const std::string& text, SDL_Color color);

    // Destructor to cleanup the background
    virtual ~MenuItem() {
        if (background) {
            SDL_FreeSurface(background);
        }
    }
};

class SimpleMenuItem : public MenuItem {
private:
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
    SimpleMenuItem(const std::string& title = "", const std::string& value = "", const std::string& path = "") 
        : MenuItem(title, value), path(path) {

        // Calculate thumbnail path here
        std::filesystem::path romPath(path);
        std::string romNameWithoutExtension = romPath.stem().string();
        std::string basePath = romPath.parent_path().string();
        thumbnailPath = basePath + "/media/images/" + romNameWithoutExtension + ".png";

    }

    ~SimpleMenuItem() {}

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

    void renderValue() const {};

};

class SubMenuMenuItem : public MenuItem {
private:
    //std::string title;
    std::unique_ptr<Menu> submenu;
    bool boolValue;

public:
    SubMenuMenuItem(const std::string& title, std::unique_ptr<Menu> submenu)
        : MenuItem(title), submenu(std::move(submenu)) {}

    SDL_Surface* getAssociatedBackground() const override;

    void executeAction() override;

    void render(SDL_Surface* screen, TTF_Font* font, int x, int y, bool isSelected) override;

    Menu* getSubMenu() const;

    std::string getName() const override;

    std::string getFolderName() const;

    void determineAndSetBackground(SDL_Surface* screen) override;

    void renderValue() const {};

};

// BooleanMenuItem

class BooleanMenuItem : public SimpleMenuItem {
private:
    bool boolValue;

public:
    BooleanMenuItem();
    BooleanMenuItem(const std::string& name, const std::string& value, bool initialValue) : SimpleMenuItem(name, value), boolValue(initialValue) {}; 

    bool getValue() const;
    void setValue(bool newValue);
    void toggleValue();
    void navigateLeft() override;
    void navigateRight() override;  
};

// MultiOptionMenuItem

class MultiOptionMenuItem : public SimpleMenuItem {
private:
    std::vector<std::string> options;  // List of available options
    int currentIndex;                  // Index of the currently selected option

public:
    MultiOptionMenuItem(const std::string& title, const std::vector<std::string>& availableOptions)
        : SimpleMenuItem(title, availableOptions.empty() ? "" : availableOptions[0]),
          options(availableOptions),
          currentIndex(0) {}

    void navigateLeft() override {
        if (currentIndex > 0) {
            currentIndex--;
            value = options[currentIndex];
        }
    }

    void navigateRight() override {
        if (currentIndex < options.size() - 1) {
            currentIndex++;
            value = options[currentIndex];
        }
    }
};

// IntegerMenuItem

class IntegerMenuItem : public SimpleMenuItem {
private:
    int maxValue;
    int minValue;
    int intValue;                  // Index of the currently selected option

public:
    //IntegerMenuItem(const std::string& name, const std::string& value, bool initialValue) : SimpleMenuItem(name, value), boolValue(initialValue) {}; 

    IntegerMenuItem(const std::string& name, const std::string& value, int min = 0, int max = 100)
        : SimpleMenuItem(name, value) {
            intValue = std::stoi(value);
            maxValue = max;
            minValue = min;
        }

    void navigateLeft() override {
        if (intValue > (minValue + 5)) {
            intValue-5;
        }
        value = std::to_string(intValue);
    }

    void navigateRight() override {
        if (intValue < (maxValue - 5)) {
            intValue+5;
        }
        value = std::to_string(intValue);
    }
};