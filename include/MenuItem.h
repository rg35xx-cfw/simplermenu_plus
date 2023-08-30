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
#include <boost/algorithm/string.hpp>

#include "Menu.h"

class Menu;
class Configuration;
enum class MenuState;

// MenuItem
// Template class for menu items

class ISettingsObserver {
 public:
  virtual ~ISettingsObserver(){};
  virtual void settingsChanged(const std::string &id, 
                               const std::string &value) = 0;
};

class ISettingsSubject {
 public:
  virtual ~ISettingsSubject(){};
  virtual void attach(ISettingsObserver *observer) = 0;
  virtual void detach(ISettingsObserver *observer) = 0;
  virtual void notifySettingsChange() = 0;
};

class MenuItem : public ISettingsSubject {
protected:
    SDL_Surface* background = nullptr;
    Menu* parentMenu = nullptr;
    std::unique_ptr<Menu> subMenu = nullptr;

    std::string id;
    std::string title;
    std::string value;

    std::list<ISettingsObserver *> observers_;

public:    
    MenuItem(const std::string& id, 
             const std::string& title, 
             const std::string& value = "", 
             std::unique_ptr<Menu> submenu = nullptr)
        : id(id), title(title), value(value), subMenu(std::move(submenu)) {}

    virtual void navigateLeft() {}
    virtual void navigateRight() {}

    virtual void executeAction() = 0;
    virtual void render(SDL_Surface* screen, TTF_Font* font, int x, int y, bool isSelected, MenuState currentState) = 0;

    virtual void renderTitle() const {};

    // TODO is this method implemented somewhere?
    //virtual void renderValue() const {};

    virtual std::string getName() const = 0;

//    virtual SDL_Surface* getAssociatedBackground() const = 0;

    virtual SDL_Surface* determineAndSetBackground(SDL_Surface* screen, MenuState currentState) {};
    
    SDL_Surface* getBackground() const {
        return background;
    }

    static SDL_Surface* loadRomBackground() {
        std::string backgroundPath = Configuration::getInstance().getThemePath() + "resources/general/background.png";
        SDL_Surface* background = IMG_Load(backgroundPath.c_str());
        if (!background) {
            std::cerr << "Failed to load ROM background: " << IMG_GetError() << std::endl;
        }
        return background;
    }

    static SDL_Surface* loadSettingsBackground() {
        std::string backgroundPath = Configuration::getInstance().getValue("Menu.homePath") + ".simplemenu/resources/settings.png";
        
        SDL_Surface* background = IMG_Load(backgroundPath.c_str());
        if (!background) {
            std::cerr << "Failed to load Settings background: " << IMG_GetError() << std::endl;
        }
        return background; 
    }

    std::string getTitle();
    std::string getValue();

    void setParentMenu(Menu* parent) {
        parentMenu = parent;
    }

    // FIXME needs to be part of a different helper/utils class
    SDL_Surface* renderText(const std::string& text, SDL_Color color);

    std::string getFolderName() const;

    bool hasSubMenu() const {
        return subMenu != nullptr;
    }

    Menu* getSubMenu() const {
        return subMenu.get();
    }

    // Destructor to cleanup the background
    virtual ~MenuItem() {
        if (background) {
            SDL_FreeSurface(background);
        }
    }

    /**
     * ISettingsObserver methods
    */
    void attach(ISettingsObserver *observer) override;
    void detach(ISettingsObserver *observer) override;
    void notifySettingsChange() override;
};

// SimpleMenuItem
// Single instance of a menu item, typically used for list entries in menus. 

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
    SDL_Surface* background = nullptr;

    // Potentially other attributes like action or callback

public:
    // Constructor with menu title, value (optional) and entry path
    // e.g.
    // title = mslug.zip
    // value = false
    // path = roms/neogeo/mslug.zip

    SimpleMenuItem(const std::string& id,
                   const std::string& title, 
                   const std::string& path, 
                   const std::string& value = "") 
        : MenuItem(id, title, value), path(path) {

        // Calculate thumbnail path here
        std::filesystem::path romPath(path);
        std::string romNameWithoutExtension = romPath.stem().string();
        std::string basePath = romPath.parent_path().string();
        thumbnailPath = basePath + "/media/images/" + romNameWithoutExtension + ".png";
    }

    // Constructor to handle submenus
    // Typically used for for
    SimpleMenuItem(const std::string& id, 
                   const std::string& title, 
                   std::unique_ptr<Menu> submenu, 
                   const std::string& path = "")
    : MenuItem(id, title, "", std::move(submenu)), path(path) {}

    ~SimpleMenuItem() {}

    const std::string& getPath() const {
        return path;
    }

    SDL_Surface* determineAndSetBackground(SDL_Surface* screen, MenuState currentState) override;

    void executeAction() override; 

    SDL_Surface* loadThumbnail();

    static void loadAliases();

    void render(SDL_Surface* screen, TTF_Font* font, int x, int y, bool isSelected, MenuState currentState) override;

    std::string getName() const override;

    bool thumbnailExists();

    void select();

    void deselect();

    void renderValue() const {};

};

// BooleanMenuItem

class BooleanMenuItem : public SimpleMenuItem {
private:
    bool boolValue;

public:
    BooleanMenuItem();
    BooleanMenuItem(const std::string& id,
                    const std::string& title, 
                    const std::string& value) 
    : SimpleMenuItem(id, 
                     title, 
                     "", 
                     boost::algorithm::to_upper_copy(value)), 
      boolValue(boost::algorithm::to_upper_copy(value) == "ON" ? true : false) {};

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

    int mod(int a, int b);

public:
    MultiOptionMenuItem(const std::string& id,
                        const std::string& title,
                        const std::string& value,
                        const std::vector<std::string>& availableOptions);

    void setValue(const std::string& value);
    void navigateLeft() override;
    void navigateRight() override;
};

// IntegerMenuItem

class IntegerMenuItem : public SimpleMenuItem {
private:
    int maxValue;
    int minValue;
    int intValue;                  // Index of the currently selected option

    void updateValuefromInt();
public:

    IntegerMenuItem(const std::string& id,
                    const std::string& name, 
                    const std::string& value, 
                    int min = 0, 
                    int max = 100);

    void navigateLeft() override;
    void navigateRight() override;
};

