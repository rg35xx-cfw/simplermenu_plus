#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <set>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include "RenderComponent.h"
#include "Configuration.h"
#include "FileManager.h"
#include "ControlMapping.h"
#include "MenuCache.h"
#include "Menu.h"
#include "HelperUtils.h"

namespace pt = boost::property_tree;

class Configuration;
class RenderComponent;

class Application {
private:
    Menu menu;
    std::vector<CachedMenuItem> allCachedItems;

    TTF_Font* font;
    SDL_Joystick *joystick = nullptr;

    Configuration cfg;

    ControlMapping controlMapping;
    RenderComponent renderComponent;

    HelperUtils helper;

    enum MenuLevel {
        MENU_SECTION,
        MENU_FOLDER,
        MENU_ROM,
        SYSTEM_SETTINGS,
        ROM_SETTINGS
    };

    MenuLevel currentMenuLevel = MENU_SECTION;

    int currentSectionIndex = 0;
    int currentFolderIndex = 0;
    int currentRomIndex = 0;
    int currentSettingsIndex = 0;

    bool isButtonHeld;
    SDL_Event lastHeldEvent;
    unsigned int repeatStartTime;
    unsigned int repeatInterval;

    void setupCache() {
        FileManager fileManager(cfg);
        MenuCache menuCache;

        // Load section groups from the section_groups folder
        auto sectionGroups = fileManager.getFiles("/userdata/system/.simplemenu/section_groups/");

        std::string cacheFilePath = "/userdata/system/simplermenu_plus/caches/global_cache.json";

        if (menuCache.cacheExists(cacheFilePath)) {
            allCachedItems = menuCache.loadFromCache(cacheFilePath);
        }

        for (const auto& sectionGroupFile : sectionGroups) {
            auto consoleDataMap = cfg.parseIniFile(
                "/userdata/system/.simplemenu/section_groups/" + sectionGroupFile);

            for (const auto& [consoleName, data] : consoleDataMap) {
                if (!menuCache.cacheExists(cacheFilePath)) {
                    for (const auto& romDir : data.romDirs) {
                        auto files = fileManager.getFiles(romDir);
                        for (const auto& file : files) {
                            std::string romPath = romDir + file;
                            allCachedItems.push_back({sectionGroupFile, consoleName, file, romPath});
                        }
                    }
                }
            }
        }

        if (!menuCache.cacheExists(cacheFilePath)) {
            menuCache.saveToCache(cacheFilePath, allCachedItems);
        }
    }

    void populateMenu(Menu& menu) {
        // Loop through the cached items and populate the Menu structure
        for (const auto& cachedItem : allCachedItems) {
            // cachedItem should have members: section, system, filename, path.

            // Check if the section already exists in the menu
            Section* section = menu.getSectionByName(cachedItem.section);
            if (!section) {
                Section newSection(cachedItem.section);
                menu.addSection(newSection);
                section = menu.getSectionByName(cachedItem.section);
            }

            // Check if the Folder already exists in the section
            Folder* folder = section->getFolderByName(cachedItem.folder);
            if (!folder) {
                Folder newFolder(cachedItem.folder);
                section->addFolder(newFolder);
                folder = section->getFolderByName(cachedItem.folder);
            }

            // Add the file to the folder
            Rom rom(cachedItem.rom, cachedItem.path);
            folder->addRom(rom);
        }
    }

    std::vector<SettingsMenuItem> loadMenuFromJSON(const std::string& jsonPath) {
        // Create a root
        pt::ptree root;

        // Load the JSON file into the property tree
        try {
            pt::read_json(jsonPath, root);
        } catch (const pt::json_parser::json_parser_error& e) {
            std::cerr << "Failed to parse " << jsonPath << ": " << e.what() << std::endl;
            return {}; // Return empty vector on error
        }

        std::vector<SettingsMenuItem> menuItems;

        // Iterate over the menu items
        for (const auto& jsonItem : root.get_child("SystemMenu")) {
            std::string id = jsonItem.second.get<std::string>("id");
            std::string type = jsonItem.second.get<std::string>("type");
            std::string title = jsonItem.second.get<std::string>("title");
            std::vector<std::string> options;

            // Check if the "options" field exists
            if (jsonItem.second.count("options")) {
                pt::ptree const& children = jsonItem.second.get_child("options");
                for (const auto& child : children) {
                    options.push_back(child.second.data());
                }
            }

            menuItems.emplace_back(id, type, title, options);
        }

        return menuItems;
    }

public:
    Application();

    void drawCurrentState();

    void handleCommand(ControlMap cmd);

    void run();

    void print_list();

    void launchRom();
};
