#pragma once

enum class MenuState;

#include <vector>
#include <iostream>
#include <string>
#include <bits/stdc++.h>
#include <memory>
#include "Configuration.h"
#include "Theme.h"
#include "MenuItem.h"


class MenuItem;
class SimpleMenuItem;
class BooleanSettingsMenu;
class MultiOptionMenuItem;

class Menu {
protected:
    std::vector<std::unique_ptr<MenuItem>> items;
    std::string title;
    int selectedItemIndex = 0;
    Menu* parent = nullptr;
    int currentPage = 0;
    int itemsPerPage;
    std::string customFontPath;
    int customFontSize;
    int itemOffset_x;
    int itemOffset_y;
    int listOffset_x;
    int listOffset_y;
    int customSpacing;
    SDL_Surface* background = nullptr;
    bool useSelectionRectangle = false; // by default, use the selection rectangle
    SDL_Color selectionRectangleColor = {30, 60, 200, 128}; // default to semi-transparent white
    int selectionRectangleWidth = 0; // default width; 0 means it will stretch to text width
    int selectionRectangleHeight = 24; // default height

    Configuration& cfg = Configuration::getInstance();
    Theme& theme = Theme::getInstance();

private:
    bool drawSelectionRectangle = false; 
    Menu* rootMenu = nullptr;

public:
    Menu(const std::string& menuTitle = "") : title(menuTitle) {
        this->itemsPerPage = theme.getIntValue("GENERAL.items");

        this->customSpacing = 0;
    }; // Updated constructor

    void setRootMenu(Menu* root) { rootMenu = root; }

    const Menu* getRootMenu() const { return rootMenu ? rootMenu : this; }
    Menu* getRootMenu() { return rootMenu ? rootMenu : this; }

    const std::string& getTitle() const { return title; } // Getter for title
    void setTitle(const std::string& newTitle) { title = newTitle; } // Setter for title


    bool isRomListMenu() const;
    void printContents(int indentLevel = 0) const;
    int startIndex = 0; //Index of the first item displayed
    int total_pages = 1;

    void print() const;

    void navigateUp();

    void navigateDown();

    void navigateLeft();

    void navigateRight();

    virtual void selectItem();

    void addItem(std::unique_ptr<SimpleMenuItem> item);

    int getNumberOfItems();

    void render(SDL_Surface* screen, TTF_Font* font, MenuState currentState);
    void render();

    int getSelectedItemIndex() const;

    void setSelectedItemIndex(int index);

    MenuItem* getItem(int index);

    void setParent(Menu* parentMenu);

    Menu* getParent() const;

    virtual bool isRomMenu() const;
    
    void handlePages();

    SDL_Surface* getBackground() const {
        return background;
    }
    
    void setBackground(const std::string& backgroundPath);

    void setFont(const std::string& fontPath, int fontSize);
    void setItemPosition(int x, int y);
    void setSpacing(int spacing);

    void setSelectionRectangleProperties(const SDL_Color& color, int width, int height);
    void enableSelectionRectangle(bool enable = true);
};

class SystemMenu : public Menu {
public:
    SystemMenu(std::string backgroundPath, std::string settingsFont);

    void specificSystemMenuActions();

};

class RomMenu : public Menu {
public:
    RomMenu();

    void specificSystemMenuActions();

    bool isRomMenu() const override;
};
