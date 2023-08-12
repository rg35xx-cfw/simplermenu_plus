#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <memory>
#include "Configuration.h"
#include "MenuItem.h"

class MenuItem;

class Menu {
protected:
    std::vector<std::unique_ptr<MenuItem>> items;
    int selectedItemIndex = 0;
    Menu* parent = nullptr;
    int currentPage = 0;
    int itemsPerPage;
    int listOffset_x;
    int listOffset_y;

public:
    Menu();

    void printContents(int indentLevel = 0) const;
    int startIndex = 0; //Index of the first item displayed
    int total_pages = 1;

    void print() const;

    void navigateUp();

    void navigateDown();

    virtual void selectItem();

    void addItem(std::unique_ptr<MenuItem> item);

    int getNumberOfItems();

    void render(SDL_Surface* screen, TTF_Font* font);

    int getSelectedItemIndex() const;

    void setSelectedItemIndex(int index);

    MenuItem* getItem(int index);

    void setParent(Menu* parentMenu);

    Menu* getParent() const;

    virtual bool isRomMenu() const;
    
    void handlePages();
};

class SystemMenu : public Menu {
public:
    SystemMenu();

    void specificSystemMenuActions();

};

class RomMenu : public Menu {
public:
    RomMenu();

    bool isRomMenu() const override;
};
