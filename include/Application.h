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
#include "MenuCache.h"
#include "Menu.h"

namespace pt = boost::property_tree;

class Configuration;
class RenderComponent;

class Application {
private:
    Menu menu;
    //Configuration cfg = Configuration::getInstance();
    std::vector<CachedMenuItem> allCachedItems;

    RenderComponent& renderComponent = RenderComponent::getInstance();

    SDL_Surface* screen;
    TTF_Font* font;
    SDL_Joystick *joystick = nullptr;
    static Application* instance;

    Configuration& cfg = Configuration::getInstance();
    Theme& theme = Theme::getInstance();

    enum MenuLevel {
        MENU_SECTION,
        MENU_FOLDER,
        MENU_ROM
    };

    MenuLevel currentMenuLevel = MENU_SECTION;

    int currentSectionIndex = 0;
    int currentFolderIndex = 0;
    int currentRomIndex = 0;

    void setupCache() {
        FileManager fileManager;
        MenuCache menuCache;

        // Load section groups from the section_groups folder
        auto sectionGroups = fileManager.getFiles("/userdata/system/.simplemenu/section_groups/");

        std::string cacheFilePath = "/userdata/system/simplermenu_plus/caches/global_cache.json";

        if (menuCache.cacheExists(cacheFilePath)) {
            allCachedItems = menuCache.loadFromCache(cacheFilePath);
        }

        for (const auto& sectionGroupFile : sectionGroups) {
            auto consoleDataMap = Configuration::getInstance().parseIniFile("/userdata/system/.simplemenu/section_groups/" + sectionGroupFile);

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


public:
    Application();

    static Application* getInstance() {
        if (!instance) {
            instance = new Application();
        }
        return instance;
    }

    void drawCurrentState();

    void handleKeyPress(SDLKey key);

    void run();

    // FIXME, these need to be moved to render utils class or similar type of helper class.
    std::string getPathWithoutExtension(const std::string& fullPath) {
        size_t dotPos = fullPath.find_last_of(".");
        size_t slashPos = fullPath.find_last_of("/\\");

        if (dotPos == std::string::npos || (slashPos != std::string::npos && dotPos < slashPos)) {
            // Return the original path if there's no dot after the last slash.
            return fullPath;
        }

        return fullPath.substr(0, dotPos);
    }

    std::string getFilenameWithoutExtension(const std::string& fullPath) {
        size_t dotPos = fullPath.find_last_of(".");
        size_t slashPos = fullPath.find_last_of("/\\");

        if (slashPos == std::string::npos) { // No slashes found
            return (dotPos == std::string::npos) ? fullPath : fullPath.substr(0, dotPos);
        }

        std::string filenameWithExtension = fullPath.substr(slashPos + 1);

        if (dotPos == std::string::npos || dotPos < slashPos) { // No dots after the last slash
            return filenameWithExtension;
        }

        return filenameWithExtension.substr(0, filenameWithExtension.find_last_of("."));
    }

    void print_list() {
        for (const auto& section : menu.getSections()) {
            std::cout << "Section: " << section.getTitle() << std::endl;
            for (const auto& folder : section.getFolders()) {
                std::cout << "  System: " << folder.getTitle() << std::endl;
                for (const auto& rom : folder.getRoms()) {
                    std::cout << "  System: " << folder.getTitle() <<  " -> Rom: " << rom.getTitle() << std::endl;
                }
            }
        }
    }

};
