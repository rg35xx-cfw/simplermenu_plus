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


Application::Application() 
    : i18n("/userdata/system/simplermenu_plus/i18n.ini"),
      cfg("/userdata/system/simplermenu_plus/config.ini"),
      theme(cfg.get(Configuration::THEME), cfg.getInt(Configuration::SCREEN_WIDTH), cfg.getInt(Configuration::SCREEN_HEIGHT)),
      controlMapping(cfg),
      renderComponent(cfg, theme),
      romSettings(cfg, i18n, this),
      systemSettings(cfg, i18n, this)
 {

    setupCache();
    populateMenu(menu);

    std::cout << "Available languages: [ ";
    std::set<std::string> langs = i18n.getLanguages();
    std::copy(langs.begin(),
              langs.end(),
              std::ostream_iterator<std::string>(std::cout, " "));
    std::cout << "]" << std::endl;

    std::cout << "Current lang: " << i18n.getLang() << std::endl;
    std::cout << i18n.get(I18n::SYSTEM_SETTINGS) << std::endl;
    std::cout << "Available settings: [ ";
    std::vector<Settings::I18nSetting> settingList = systemSettings.getSystemSettings();
    for (const auto& setting : settingList) {
        std::cout << setting.title << ":" << setting.value << ", ";
    }
    std::cout << "]" << std::endl;

    i18n.setLang("SPANISH");
    std::cout << "Current lang: " << i18n.getLang() << std::endl;
    std::cout << i18n.get(I18n::SYSTEM_SETTINGS) << std::endl;
    std::cout << "Available settings: [ ";
    settingList = systemSettings.getSystemSettings();
    for (const auto& setting : settingList) {
        std::cout << setting.title << ":" << setting.value << ", ";
    }
    std::cout << "]" << std::endl;

    i18n.setLang("ENGLISH");
    std::cout << "Current lang: " << i18n.getLang() << std::endl;
    std::cout << i18n.get(I18n::SYSTEM_SETTINGS) << std::endl;
    std::cout << "Available settings: [ ";
    settingList = systemSettings.getSystemSettings();
    for (const auto& setting : settingList) {
        std::cout << setting.title << ":" << setting.value << ", ";
    }
    std::cout << "]" << std::endl;

    theme.loadTheme(cfg.get(Configuration::THEME), cfg.getInt(Configuration::SCREEN_WIDTH), cfg.getInt(Configuration::SCREEN_HEIGHT));

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
    switch (currentMenuLevel) {
        case MENU_SECTION:
        {
            std::string sectionName = menu.getSections()[currentSectionIndex].getTitle();

            int numberOfFolders = menu.getSections()[currentSectionIndex].getFolders().size();
            renderComponent.drawSection(sectionName, numberOfFolders);
            break;
        }
        case MENU_FOLDER:
        {
            std::string folderName = menu.getSections()[currentSectionIndex].getFolders()[currentFolderIndex].getTitle();
            std::string folderPath = "";
            int numberOfRoms = menu.getSections()[currentSectionIndex].getFolders()[currentFolderIndex].getRoms().size();
            renderComponent.drawFolder(folderName, folderPath, numberOfRoms);
            break;
        }
        case MENU_ROM:
        {
            std::vector<std::pair<std::string, std::string>> romData;
            for (const Rom& rom : menu.getSections()[currentSectionIndex].getFolders()[currentFolderIndex].getRoms()) {
                romData.push_back({rom.getTitle(), rom.getPath()});
            }
            renderComponent.drawRomList(menu.getSections()[currentSectionIndex].getFolders()[currentFolderIndex].getTitle(), romData, currentRomIndex);
            break;
        }
        case SYSTEM_SETTINGS:
        {
            renderComponent.drawSystemSettings(i18n.get(I18n::SYSTEM_SETTINGS), systemSettings.getSystemSettings(), currentSettingsIndex);
            break;
        }
        case ROM_SETTINGS:
        {
            renderComponent.drawRomSettings(i18n.get(I18n::ROM_SETTINGS), romSettings.getRomSettings(), currentRomSettingsIndex);
            break;            
        }
    }
}

void Application::handleCommand(ControlMap cmd) {
    switch (currentMenuLevel) {
        case MENU_SECTION:
            if (cmd == CMD_ENTER) { // ENTER
                currentMenuLevel = MENU_FOLDER;
                currentFolderIndex = 0;
                renderComponent.resetValues();
            } else if (cmd == CMD_UP) { // UP
                if (currentSectionIndex > 0) currentSectionIndex--;
                else currentSectionIndex = menu.getSections().size() - 1;
            } else if (cmd == CMD_DOWN) { // DOWN
                currentSectionIndex = (currentSectionIndex + 1) % menu.getSections().size();
            }
            break;
            
        case MENU_FOLDER:
            if (cmd == CMD_ENTER) { // KEY_A/ENTER
                currentMenuLevel = MENU_ROM;
                currentRomIndex = 0;
                renderComponent.resetValues();
                romSettings.getCores(menu.getSections()[currentSectionIndex].getTitle(), menu.getSections()[currentSectionIndex].getFolders()[currentFolderIndex].getTitle() );
            } else if (cmd == CMD_BACK) { // KEY_B/ESC
                currentMenuLevel = MENU_SECTION;
                renderComponent.resetValues();
            } else if (cmd == CMD_UP) { // UP
                const Section& section = menu.getSections()[currentSectionIndex];
                if (currentFolderIndex > 0) currentFolderIndex--;
                else currentFolderIndex = section.getFolders().size() - 1;
            } else if (cmd == CMD_DOWN) { // DOWN
                const Section& section = menu.getSections()[currentSectionIndex];
                currentFolderIndex = (currentFolderIndex + 1) % section.getFolders().size();
            }
            break;
            
        case MENU_ROM:
            if (cmd == CMD_BACK) { // ESC
                currentMenuLevel = MENU_FOLDER;
                renderComponent.resetValues();
            } else if (cmd == CMD_UP) { // UP
                const Folder& folder = menu.getSections()[currentSectionIndex].getFolders()[currentFolderIndex];
                if (currentRomIndex > 0) currentRomIndex--;
                else currentRomIndex = folder.getRoms().size() - 1;
            } else if (cmd == CMD_DOWN) { // DOWN
                const Folder& folder = menu.getSections()[currentSectionIndex].getFolders()[currentFolderIndex];
                currentRomIndex = (currentRomIndex + 1) % folder.getRoms().size();
            } else if (cmd == CMD_ENTER) { // ENTER
                std::cout << "execute rom" << std::endl;
                launchRom();
                renderComponent.resetValues();
            } else if (cmd == CMD_ROM_SETTINGS) {
                currentMenuLevel = ROM_SETTINGS;
                renderComponent.resetValues();
            }
            break;
        case SYSTEM_SETTINGS:
            if (cmd == CMD_BACK) { // ESC
                currentMenuLevel = MENU_FOLDER;
                renderComponent.resetValues();
            } else if (cmd == CMD_UP) { // UP
                if (currentSettingsIndex > 0) currentSettingsIndex--;
                else currentSettingsIndex = cfg.getSectionSize(Configuration::SYSTEM) - 1;
            } else if (cmd == CMD_DOWN) { // DOWN
                currentSettingsIndex = (currentSettingsIndex + 1) % (cfg.getSectionSize(Configuration::SYSTEM));
                std::cout << "currentSettingsIndex: " << currentSettingsIndex << std::endl;
            }
            break;
        case ROM_SETTINGS:
            if (cmd == CMD_BACK) { // ESC
                currentMenuLevel = MENU_ROM;
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
        currentMenuLevel = SYSTEM_SETTINGS;
        renderComponent.resetValues();
    }

    if(currentMenuLevel == SYSTEM_SETTINGS) {
        if (cmd == CMD_UP) {
            systemSettings.navigateUp();
        } else if (cmd == CMD_DOWN) {
            systemSettings.navigateDown();
        } else if (cmd == CMD_LEFT) {
            systemSettings.navigateLeft();
        } else if (cmd == CMD_RIGHT) {
            systemSettings.navigateRight();
        } else if (cmd == CMD_ENTER) {
            systemSettings.navigateEnter();
        }

        std::string currentKey = systemSettings.getCurrentKey();
        std::string currentValue = systemSettings.getCurrentValue();

    }

    if(currentMenuLevel == ROM_SETTINGS) {
        if (cmd == CMD_UP) {
            romSettings.navigateUp();
        } else if (cmd == CMD_DOWN) {
            romSettings.navigateDown();
        } else if (cmd == CMD_LEFT) {
            romSettings.navigateLeft();
        } else if (cmd == CMD_RIGHT) {
            romSettings.navigateRight();
        } else if (cmd == CMD_ENTER) {
            romSettings.navigateEnter();
        }

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

std::vector<std::string> Application::getCores() {

    std::string folderName = menu.getSections()[currentSectionIndex].getFolders()[currentFolderIndex].getTitle();
    std::string sectionName = menu.getSections()[currentSectionIndex].getTitle();

    std::map<std::string, ConsoleData> consoleDataMap = cfg.parseIniFile(cfg.get(Configuration::HOME_PATH) + ".simplemenu/section_groups/" + sectionName);

    std::vector<std::string> cores;

    // Check if the parentTitle exists in the consoleDataMap
    if (consoleDataMap.find(folderName) != consoleDataMap.end()) {
        // Access the ConsoleData for the parentTitle
        ConsoleData consoleData = consoleDataMap[folderName];

        // Check if the execs vector is not empty
        if (!consoleData.execs.empty()) {
            cores = consoleData.execs;
        }
    }

    return cores;
}

void Application::launchRom() {

    std::string romName = menu.getSections()[currentSectionIndex].getFolders()[currentFolderIndex].getRoms()[currentRomIndex].getTitle();
    std::string romPath = menu.getSections()[currentSectionIndex].getFolders()[currentFolderIndex].getRoms()[currentRomIndex].getPath();
    std::string folderName = menu.getSections()[currentSectionIndex].getFolders()[currentFolderIndex].getTitle();
    std::string sectionName = menu.getSections()[currentSectionIndex].getTitle();
    std::cout << "Launching rom: " << sectionName << " -> " << folderName << " -> " << romName << std::endl;

    std::map<std::string, ConsoleData> consoleDataMap = cfg.parseIniFile(cfg.get(Configuration::HOME_PATH) + ".simplemenu/section_groups/" + sectionName);

    // We are using the last selected core for a given system, by default it's the first available core only another 
    // core has been selected in rom settings -> core override
    std::string execLauncher = cfg.get(Configuration::HOME_PATH) + ".simplemenu/launchers/" + cfg.get(Configuration::CORE_OVERRIDE);

    std::vector<std::string> cores = getCores();

/*
    // Check if the parentTitle exists in the consoleDataMap
    if (consoleDataMap.find(folderName) != consoleDataMap.end()) {
        // Access the ConsoleData for the parentTitle
        ConsoleData consoleData = consoleDataMap[folderName];

        // Check if the execs vector is not empty
        //if (!consoleData.execs.empty()) {
            // Retrieve the first exec string
            execLauncher = cores[1];//consoleData.execs.front();
        //}
    }
*/

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
// SettingsObserver methods

void Application::settingsChanged(const std::string& key, const std::string& value) {
    std::cout << key << " changed to " << value << std::endl;
    if (key == Configuration::LANGUAGE) {
        i18n.setLang(value);
    } else if (key == Configuration::THEME) {
        theme.loadTheme(value, cfg.getInt(Configuration::SCREEN_WIDTH), cfg.getInt(Configuration::SCREEN_HEIGHT));
    }
    cfg.set(key, value);
}

std::string Application::getName() {
    return "Application::" + std::to_string((unsigned long long)(void**)this);
}

//
/////////////////
