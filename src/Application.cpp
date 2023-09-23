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

Application* Application::instance = nullptr;

Application::Application() {
    setupCache();
    populateMenu(menu);

    instance = this;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        // Handle error
        exit(1);
    }

    screen = SDL_SetVideoMode(
        Configuration::getInstance().getIntValue(SettingId::SCREEN_WIDTH),
        Configuration::getInstance().getIntValue(SettingId::SCREEN_HEIGHT),
        Configuration::getInstance().getIntValue(SettingId::SCREEN_DEPTH),
        SDL_HWSURFACE | SDL_DOUBLEBUF);

    if (!screen) {
        std::cerr << "Unable to set video mode: " << SDL_GetError() << std::endl;
        SDL_Quit();
        exit(1);
    }

    if (TTF_Init() == -1) {
        std::cerr << "Unable to initialize TTF: " << TTF_GetError() << std::endl;
        SDL_Quit();
        exit(1);
    }

    font = TTF_OpenFont(
    theme.getValue("GENERAL.font", true).c_str(),
    theme.getIntValue("GENERAL.font_size"));
    TTF_SetFontHinting(font, TTF_HINTING_NORMAL);  // or TTF_HINTING_LIGHT, TTF_HINTING_MONO, TTF_HINTING_NONE
    TTF_SetFontKerning(font, 1); // 1 to enable, 0 to disable

    // Enable keyboard repeat (only for keys, not for buttons)
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

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

    renderComponent.initialize(screen, font);
}

void Application::drawCurrentState() {
    std::stringstream ss;
    switch (currentMenuLevel) {
        case MENU_SECTION:
        {
            std::string sectionName = menu.getSections()[currentSectionIndex].getTitle();

            std::string sectionBackgroundPath = Configuration::getInstance().getValue(SettingId::THEME_PATH) + 
                                 std::to_string(Configuration::getInstance().getIntValue(SettingId::SCREEN_WIDTH)) + "x" +
                                 std::to_string(Configuration::getInstance().getIntValue(SettingId::SCREEN_HEIGHT)) + "/" +
                                 Configuration::getInstance().getValue(SettingId::THEME_NAME) + "/" +
                                 theme.getValue("GENERAL.section_groups_folder") + getFilenameWithoutExtension(sectionName) + ".png";

            int numberOfFolders = menu.getSections()[currentSectionIndex].getFolders().size();
            renderComponent.drawSection(sectionName, sectionBackgroundPath, numberOfFolders);
            break;
        }
        case MENU_FOLDER:
        {
            std::string folderName = menu.getSections()[currentSectionIndex].getFolders()[currentFolderIndex].getTitle();
            std::string folderPath = "";
            int numberOfRoms = menu.getSections()[currentSectionIndex].getFolders()[currentFolderIndex].getRoms().size();
            renderComponent.drawSystem(folderName, folderPath, numberOfRoms);
            break;
        }
        case MENU_ROM:
            std::vector<std::pair<std::string, std::string>> romData;
            for (const Rom& rom : menu.getSections()[currentSectionIndex].getFolders()[currentFolderIndex].getRoms()) {
                romData.push_back({rom.getTitle(), rom.getPath()});
            }
            renderComponent.drawRomList(romData, currentRomIndex);
            break;
    }
}

void Application::handleKeyPress(SDLKey key) {
    switch (currentMenuLevel) {
        case MENU_SECTION:
            if (key == 13) { // ENTER
                currentMenuLevel = MENU_FOLDER;
                currentFolderIndex = 0;
            } else if (key == 273) { // UP
                if (currentSectionIndex > 0) currentSectionIndex--;
                else currentSectionIndex = menu.getSections().size() - 1;
            } else if (key == 274) { // DOWN
                currentSectionIndex = (currentSectionIndex + 1) % menu.getSections().size();
            }
            break;
            
        case MENU_FOLDER:
            if (key == 13) { // ENTER
                currentMenuLevel = MENU_ROM;
                currentRomIndex = 0;
            } else if (key == 27) { // ESC
                currentMenuLevel = MENU_SECTION;
            } else if (key == 273) { // UP
                const Section& section = menu.getSections()[currentSectionIndex];
                if (currentFolderIndex > 0) currentFolderIndex--;
                else currentFolderIndex = section.getFolders().size() - 1;
            } else if (key == 274) { // DOWN
                const Section& section = menu.getSections()[currentSectionIndex];
                currentFolderIndex = (currentFolderIndex + 1) % section.getFolders().size();
            }
            break;
            
        case MENU_ROM:
            if (key == 27) { // ESC
                currentMenuLevel = MENU_FOLDER;
            } else if (key == 273) { // UP
                const Folder& folder = menu.getSections()[currentSectionIndex].getFolders()[currentFolderIndex];
                if (currentRomIndex > 0) currentRomIndex--;
                else currentRomIndex = folder.getRoms().size() - 1;
            } else if (key == 274) { // DOWN
                const Folder& folder = menu.getSections()[currentSectionIndex].getFolders()[currentFolderIndex];
                currentRomIndex = (currentRomIndex + 1) % folder.getRoms().size();
            } else if (key == 13) { // ENTER
                std::cout << "execute rom" << std::endl;
            }
            break;
    }
}

void Application::run() {
    bool isRunning = true;
    SDL_Event event;

    int fps = 0;
    int frameCount = 0;
    Uint32 fpsTimer = 0;
    //Uint32 msgDelay = 0;

    Uint32 frameStart = 0;

    // print_list();

    //Uint32 currentTime = SDL_GetTicks();

    while (isRunning) {
        int screenRefresh = 30;
        
        int frameDelay = 1000 / screenRefresh;

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
                    handleKeyPress(event.key.keysym.sym);
                    break;
                case SDL_JOYAXISMOTION:
                case SDL_JOYBUTTONDOWN:
                case SDL_JOYBUTTONUP:
                case SDL_JOYHATMOTION:
                    break;
                case SDL_KEYUP:
                    break;
            }
        }

        // Handle FPS information
        if (SDL_GetTicks() - fpsTimer >= 1000) {
            fps = frameCount;
            frameCount = 0;
            fpsTimer = SDL_GetTicks();
        }

        drawCurrentState();

        renderComponent.printFPS(fps);

        if (SDL_Flip(screen) == -1) {
            std::cerr << "SDL_Flip failed: " << SDL_GetError() << std::endl;
            return;  // or handle the error as appropriate
        }

        frameCount++;
    }
}

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
