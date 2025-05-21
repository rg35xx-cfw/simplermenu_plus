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

#include "Application.h"
#include "Exception.h"


Application::Application() 
    : i18n("/userdata/system/configs/simplermenu_plus/i18n.ini"),
      cfg("/userdata/system/configs/simplermenu_plus/config.ini", 
          "/userdata/system/configs/simplermenu_plus/.state"),
      theme(cfg.get(Configuration::HOME_PATH), cfg.get(Configuration::THEME_PATH), cfg.get(Configuration::THEME), cfg.getInt(Configuration::SCREEN_WIDTH), cfg.getInt(Configuration::SCREEN_HEIGHT)),
      controlMapping(cfg),
      renderComponent(cfg, theme),
      appSettings(cfg, i18n, 0, 100, 5),
      systemSettings(cfg, i18n, 0, 100, 5),
      romSettings(cfg, i18n, 0, 100, 5)
 {

    // Observe settings changes
    appSettings.attach(this);
    systemSettings.attach(this);
    romSettings.attach(this);

    // Language observers
    attach(&romSettings);

    romSettings.initializeSettings();
    systemSettings.initializeSettings();
    appSettings.initializeSettings();

    bool rebuildCache = false;
    try {
        state = cfg.loadState();
        std::cout << "State loaded: " << state.currentMenuLevel << std::endl;
        std::cout << "Folder: " << state.currentFolderIndex << std::endl;
        std::cout << "Rom: " << state.currentRomIndex << std::endl;

        // Restore cache if coming back from a launcher callback
        if (!state.launcherCallback) {
            std::cout << "Regular launch (no callback)" << std::endl;
            rebuildCache = true;
        }

    } catch (const StateNotFoundException& e) {
        std::cout << "State not found, using default values" << std::endl;
        state.currentMenuLevel = MenuLevel::MENU_SYSTEM;
        state.currentFolderIndex = 0;
        state.currentRomIndex = 0;
        state.launcherCallback = false;

        cfg.saveState(state);

        rebuildCache = true;
        
    }

    if (rebuildCache) {
        // Initialize/Rebuild the cache - always create a new cache 
        // from disk contents on startup
        std::cout << "Rebuilding cache" << std::endl; 
        loadCache(true);

    } else {
        std::cout << "Loading cache from disk" << std::endl;
        loadCache(false);
        
    }

    if (state.launcherCallback) {
        // If we are coming from a launcher callback, we need to reset the state
        std::cout << "Launcher callback processed" << std::endl;
        state.launcherCallback = false;
        cfg.saveState(state);
        
    }

    populateMenu(menu);

    theme.loadTheme(cfg.get(Configuration::HOME_PATH), cfg.get(Configuration::THEME_PATH), cfg.get(Configuration::THEME), cfg.getInt(Configuration::SCREEN_WIDTH), cfg.getInt(Configuration::SCREEN_HEIGHT));

    renderComponent.initialize();

    // Initialize joystick
    if (SDL_Init(SDL_INIT_JOYSTICK) < 0) {
        std::cerr << "Failed to initialize SDL joystick subsystem: " << SDL_GetError() << std::endl;
    }

    if (SDL_NumJoysticks() > 0) {
        joystick = SDL_JoystickOpen(0);
        if (!joystick) {
            std::cerr << "Failed to open joystick: " << SDL_GetError() << std::endl;
            // Handle the error appropriately.
        } else {
            std::cout << "Joystick Name: " << SDL_JoystickName(0) << std::endl;
            std::cout << "Number of Axes: " << SDL_JoystickNumAxes(joystick) << std::endl;
            std::cout << "Number of Buttons: " << SDL_JoystickNumButtons(joystick) << std::endl;
        }
    }
    
}

void Application::drawCurrentState() {
    std::stringstream ss;
    switch (state.currentMenuLevel) {
        case MENU_SYSTEM:
        {
            std::string systemName = menu.getSystems()[state.currentFolderIndex].getTitle();
            std::string systemPath = "";
            int numberOfRoms = menu.getSystems()[state.currentFolderIndex].getRoms().size();
            renderComponent.drawSystem(systemName, systemPath, numberOfRoms);
            break;
        }
        case MENU_ROM:
        {
            std::vector<std::pair<std::string, std::string>> romData;
            for (const Rom& rom : menu.getSystems()[state.currentFolderIndex].getRoms()) {
                romData.push_back({rom.getTitle(), rom.getPath()});
            }
            renderComponent.drawRomList(menu.getSystems()[state.currentFolderIndex].getTitle(), romData, state.currentRomIndex);
            break;
        }
        case APP_SETTINGS:
        {
            renderComponent.drawSettingsMenu("Settings", appSettings.getAppSettings(), currentSettingsIndex, cfg.getSectionSize(Configuration::APPLICATION));

            break;
        }
        case SYSTEM_SETTINGS:
        {
            renderComponent.drawSettingsMenu("System Settings", systemSettings.getSystemSettings(), currentFolderSettingsIndex, cfg.getSectionSize(Configuration::SYSTEM));
            break; 
        }
        case ROM_SETTINGS:
        {
            renderComponent.drawSettingsMenu("Game Settings", romSettings.getRomSettings(), currentRomSettingsIndex, cfg.getSectionSize(Configuration::GAME));
            break;
        }
    }
}

void Application::handleCommand(ControlMap cmd) {
    switch (state.currentMenuLevel) {
        case MenuLevel::MENU_SYSTEM:
            if (cmd == CMD_ENTER) { // KEY_A/ENTER
                state.currentMenuLevel = MenuLevel::MENU_ROM;
                state.currentRomIndex = 0;
                renderComponent.resetValues();
            } else if (cmd == CMD_UP) { // UP
                const System& system = menu.getSystems()[state.currentFolderIndex];
                if (state.currentFolderIndex > 0) state.currentFolderIndex--;
                else state.currentFolderIndex = menu.getSystems().size() - 1;
            } else if (cmd == CMD_DOWN) { // DOWN
                const System& system = menu.getSystems()[state.currentFolderIndex];
                state.currentFolderIndex = (state.currentFolderIndex + 1) % menu.getSystems().size();
            } else if (cmd == CMD_ROM_SETTINGS) {
                state.currentMenuLevel = MenuLevel::SYSTEM_SETTINGS;
                renderComponent.resetValues();
            }

            // Save state after navigating, but not when entering the ROM settings
            if (cmd != CMD_ROM_SETTINGS) {
                cfg.saveState(state);
            }

            break;
        case MenuLevel::MENU_ROM:
            if (cmd == CMD_BACK) { // ESC
                state.currentMenuLevel = MenuLevel::MENU_SYSTEM;
                renderComponent.resetValues();
            } else if (cmd == CMD_UP) { // UP
                const System& system = menu.getSystems()[state.currentFolderIndex];
                if (state.currentRomIndex > 0) state.currentRomIndex--;
                else state.currentRomIndex = system.getRoms().size() - 1;
            } else if (cmd == CMD_DOWN) { // DOWN
                const System& system = menu.getSystems()[state.currentFolderIndex];
                state.currentRomIndex = (state.currentRomIndex + 1) % system.getRoms().size();
            } else if (cmd == CMD_ENTER) { // ENTER
                std::cout << "execute rom" << std::endl;
                launchRom();
                renderComponent.resetValues();
            } else if (cmd == CMD_ROM_SETTINGS) {
                state.currentMenuLevel = MenuLevel::ROM_SETTINGS;
                renderComponent.resetValues();
                romSettings.getCores(menu.getSystems()[state.currentFolderIndex].getTitle(), cache);
            }

            // Save state after navigating, but not when entering the ROM settings
            // When launching a rom we should never get back here in any case
            if (cmd != CMD_ROM_SETTINGS) {
                cfg.saveState(state);
            }

            break;
        case APP_SETTINGS:
            if (cmd == CMD_BACK) { // ESC
                state = cfg.loadState();
                renderComponent.resetValues();
            } else if (cmd == CMD_UP) { // UP
                if (currentSettingsIndex > 0) currentSettingsIndex--;
                else currentSettingsIndex = cfg.getSectionSize(Configuration::APPLICATION) - 1;
            } else if (cmd == CMD_DOWN) { // DOWN
                currentSettingsIndex = (currentSettingsIndex + 1) % (cfg.getSectionSize(Configuration::APPLICATION));
                std::cout << "currentSettingsIndex: " << currentSettingsIndex << std::endl;
            }
            break;
        case SYSTEM_SETTINGS:
            if (cmd == CMD_BACK) { // ESC
                state.currentMenuLevel = MenuLevel::MENU_SYSTEM;
                renderComponent.resetValues();
            } else if (cmd == CMD_UP) { // UP
                if (state.currentFolderIndex > 0) state.currentFolderIndex--;
                else state.currentFolderIndex = cfg.getSectionSize(Configuration::SYSTEM) - 1;
            } else if (cmd == CMD_DOWN) { // DOWN
                state.currentFolderIndex = (state.currentFolderIndex + 1) % (cfg.getSectionSize(Configuration::SYSTEM));
                std::cout << "currentSettingsIndex: " << state.currentFolderIndex << std::endl;
            }
            break;
        case ROM_SETTINGS:
            if (cmd == CMD_BACK) { // ESC
                state.currentMenuLevel = MenuLevel::MENU_ROM;
                renderComponent.resetValues();
            } else if (cmd == CMD_UP) { // UP
                if (currentRomSettingsIndex > 0) currentRomSettingsIndex--;
                else currentRomSettingsIndex = cfg.getSectionSize(Configuration::GAME) - 1;
            } else if (cmd == CMD_DOWN) { // DOWN
                currentRomSettingsIndex = (currentRomSettingsIndex + 1) % (cfg.getSectionSize(Configuration::GAME));
            } 
            break;
    }

    if (cmd == CMD_SYS_SETTINGS) {
        if(state.currentMenuLevel != MenuLevel::APP_SETTINGS) {
            cfg.saveState(state);
        }
        state.currentMenuLevel = MenuLevel::APP_SETTINGS;
        renderComponent.resetValues();
    }

    if(state.currentMenuLevel == MenuLevel::APP_SETTINGS) {
        if (cmd == CMD_UP) {
            appSettings.navigateUp();
        } else if (cmd == CMD_DOWN) {
            appSettings.navigateDown();
        } else if (cmd == CMD_LEFT) {
            appSettings.navigateLeft();
        } else if (cmd == CMD_RIGHT) {
            appSettings.navigateRight();
        } else if (cmd == CMD_ENTER) {
            appSettings.navigateEnter();
        }

        std::string currentKey = appSettings.getCurrentKey();
        std::string currentValue = appSettings.getCurrentValue();

    }

    if(state.currentMenuLevel == SYSTEM_SETTINGS) {
        if (cmd == CMD_UP) {
            appSettings.navigateUp();
        } else if (cmd == CMD_DOWN) {
            appSettings.navigateDown();
        } else if (cmd == CMD_LEFT) {
            appSettings.navigateLeft();
        } else if (cmd == CMD_RIGHT) {
            appSettings.navigateRight();
        } else if (cmd == CMD_ENTER) {
            appSettings.navigateEnter();
        }

        std::string currentKey = appSettings.getCurrentKey();
        std::string currentValue = appSettings.getCurrentValue();

    }

    if(state.currentMenuLevel == ROM_SETTINGS) {
        if (cmd == CMD_UP) {
            romSettings.navigateUp();
        } else if (cmd == CMD_DOWN) {
            romSettings.navigateDown();
        } else if (cmd == CMD_LEFT) {
            romSettings.navigateLeft();
        } else if (cmd == CMD_RIGHT) {
            romSettings.navigateRight();
        }// } else if (cmd == CMD_ENTER) {
        //     romSettings.navigateEnter();
        // }

        std::string currentKey = romSettings.getCurrentKey();
        std::string currentValue = romSettings.getCurrentValue();

    }
}

bool Application::isInteger(const std::string &s) {
    return !s.empty() && std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

void Application::run() {
    bool isRunning = true;
    SDL_Event event;

    int fps = 0;
    int frameCount = 0;
    Uint32 fpsTimer = 0;

    Uint32 frameStart = 0;

    while (isRunning) {
        int screenRefresh = cfg.getInt(Configuration::SCREEN_REFRESH);
        
        Uint32 frameDelay = 1000 / screenRefresh;

        // Wait if last frame was drawn too fast
        if (SDL_GetTicks() - frameStart < frameDelay) {
            continue;
        }

        // // Fine tune FPS
        if (frameCount == screenRefresh && ((SDL_GetTicks() - fpsTimer) < 1000)) {
            continue;
        }

        frameStart = SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    isRunning = false;
                    break;
                case SDL_KEYDOWN:
                case SDL_JOYAXISMOTION:
                case SDL_JOYBUTTONDOWN:
                case SDL_JOYHATMOTION:
                    isButtonHeld = true;
                    lastHeldEvent = event;
                    repeatStartTime = SDL_GetTicks() + 500;
                    repeatInterval = 100;
                    handleCommand(controlMapping.convertCommand(event));
                    break;
                case SDL_JOYBUTTONUP:
                case SDL_KEYUP:
                    isButtonHeld = false;
                    break;
            }
        }

        if (isButtonHeld && SDL_GetTicks() > repeatStartTime) {
            handleCommand(controlMapping.convertCommand(lastHeldEvent));
            repeatStartTime = SDL_GetTicks() + repeatInterval;
        }

        // Handle FPS information
        if (SDL_GetTicks() - fpsTimer >= 1000) {
            fps = frameCount;
            frameCount = 0;
            fpsTimer = SDL_GetTicks();
        }

        drawCurrentState();

        renderComponent.printFPS(fps);

        renderComponent.update();

        frameCount++;
    }
}

void Application::print_list() {
    for (const auto& system : menu.getSystems()) {
        std::cout << "  System: " << system.getTitle() << std::endl;
        for (const auto& rom : system.getRoms()) {
            std::cout << "  System: " << system.getTitle() <<  " -> Rom: " << rom.getTitle() << std::endl;
        }
    }
}

void Application::launchRom() {

    // Save application state first and mark it as a launcher callback
    state.launcherCallback = true;
    cfg.saveState(state);

    std::string romName = menu.getSystems()[state.currentFolderIndex].getRoms()[state.currentRomIndex].getTitle();
    std::string romPath = menu.getSystems()[state.currentFolderIndex].getRoms()[state.currentRomIndex].getPath();
    std::string systemName = menu.getSystems()[state.currentFolderIndex].getTitle();
    std::cout << "Launching rom: " << systemName << " -> " << romName << std::endl;

    std::string execLauncher = cfg.get(Configuration::HOME_PATH) + "launchers/" + cache.getMenuItemByPath(romPath).core;

    // Launch emulator
    std::string command = execLauncher + " '" + romPath + "'";
    std::cout << "Executing: " << command << std::endl;

    setenv("SDL_NOMOUSE", "1", 1);

    pid_t pid = fork();
    if (pid == 0) {
            execlp("launcher.sh","launcher.sh", execLauncher.c_str(), romPath.c_str(), NULL);
            exit(1);
    } else if (pid > 0) {
            SDL_Quit();
            exit(0);
    } else {
            std::cerr << "Fork failed" << std::endl;
    }

    // Exit the application to free all resources
    SDL_Quit();
    exit(0);
}

/////////////////
// ISettingsObserver methods

void Application::settingsChanged(const std::string& key, const std::string& value) {
    std::cout << key << " changed to " << value << std::endl;
    if (key == Configuration::LANGUAGE) {
        i18n.setLang(value);
        notifyLanguageChange();

    } else if (key == Configuration::THEME) {
        theme.loadTheme(cfg.get(Configuration::HOME_PATH), cfg.get(Configuration::THEME_PATH), value, cfg.getInt(Configuration::SCREEN_WIDTH), cfg.getInt(Configuration::SCREEN_HEIGHT));
    } else if (key == Configuration::CORE_OVERRIDE) {
        std::cout << "Calling CORE OVERRIDE " << std::endl;
        
        if (state.currentMenuLevel == ROM_SETTINGS) {
            std::string romPath = menu.getSystems()[state.currentFolderIndex].getRoms()[state.currentRomIndex].getPath();

            if (romPath != "") {
                 cache.menuCacheUpdateItem(
                    cfg.get(Configuration::HOME_PATH) + "/" + cfg.get(Configuration::GLOBAL_CACHE), 
                    romPath, value);
                cache.systemCacheUpdateSelectedExec(
                    cfg.get(Configuration::HOME_PATH) + "systems.json", 
                    menu.getSystems()[state.currentFolderIndex].getTitle(), value);
            }

        }
    } 
    
    else if (key == Configuration::QUIT) {
        if(value != "INTERNAL") {
            SDL_Quit();
            exit(0);
        }
    }
    cfg.set(key, value);
    cfg.saveConfigIni();
}

/////////////////
// ILanguageSubject methods

void Application::attach(ILanguageObserver *observer) {
    langObservers.push_back(observer);
    std::cout << "LangObserver added to " << getName() 
              << " object: " << observer->getName() << "\n";
}

void Application::detach(ILanguageObserver *observer) {
    langObservers.erase(std::remove(langObservers.begin(), 
                                    langObservers.end(), 
                                    observer), 
                        langObservers.end());
}

void Application::notifyLanguageChange() {
    for (ILanguageObserver *observer : langObservers) {
        observer->languageChanged();
        std::cout << "LangObserver " << observer->getName() << " notified by " 
                  << getName() << std::endl;
    }
}

/////////////////
// ISettingsObserver and ILanguageSubject common methods

std::string Application::getName() {
    return "Application::" + std::to_string((unsigned long long)(void**)this);
}

//
/////////////////


/////////////////
// Private methods

void Application::loadCache(bool force) {
   
    // Initialize menu cache field
    Cache cache;

    // Get the path to the cache file from config.ini file
    std::string cacheFilePath = cfg.get(Configuration::HOME_PATH) + "/" + cfg.get(Configuration::GLOBAL_CACHE);

    if (force || !cache.menuCacheExists(cacheFilePath)) {
        // Cache does not exist or force update is requested:
        // Read all sections and create a new cache

        std::cout << "Force cache update" << std::endl;
        
        // get the path to the cache file by removing the filename
        // from the cacheFilePath
        std::filesystem::path cacheFilePathObj(cacheFilePath);
        cacheFilePathObj.remove_filename();
        // create the directories if they do not exist
        std::filesystem::create_directories(cacheFilePathObj.string());

        cache.menuCacheSave(cacheFilePath, populateCache());

    } else {

        std::cout << "Cache exists, loading from cache file" << std::endl;
        // Ignore the return value as we are not using it here, just
        // load the cache file contents into the in-memory cache
        cache.menuCacheLoad(cacheFilePath);

    }

}

std::vector<CachedMenuItem> Application::populateCache() {
    FileManager fileManager(cfg);

    // Load systems from systems.json
    auto consoleDataMap = cache.systemsCacheLoad(cfg.get(Configuration::HOME_PATH) + "systems.json");
    std::string romsPath = cfg.get(Configuration::ROMS_PATH);

    std::vector<CachedMenuItem> allCachedItems;

    for (const auto& [consoleName, data] : consoleDataMap) {
        for (const auto& romDir : data.romDirs) {
            auto files = fileManager.getFiles(romsPath + romDir);
            for (const auto& file : files) {
                std::string romPath = romsPath + "/" + romDir + "/" + file;
                allCachedItems.push_back({consoleName, file, romPath});
            }
        }
    }

    return allCachedItems;
}

void Application::populateMenu(Menu& menu) {
    // Loop through the cached items and populate the Menu structure
    for (const auto& cachedItem : cache.menuCacheLoad(cfg.get(Configuration::HOME_PATH) + "/" + cfg.get(Configuration::GLOBAL_CACHE))) {
        // cachedItem should have members: system, filename, path.

        // Check if the System already exists in the menu
        System* system = menu.getSystemByName(cachedItem.system);
        if (!system) {
            System newSystem(cachedItem.system);
            menu.addSystem(newSystem);
            system = menu.getSystemByName(cachedItem.system);
        }

        // Add the file to the system
        Rom rom(cachedItem.rom, cachedItem.path);
        system->addRom(rom);
    }
}