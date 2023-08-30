#include <memory>
#include <string>
#include <fstream>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "Configuration.h"
#include "Menu.h"
#include "MenuItem.h"
#include "ThumbnailCache.h"
#include "State.h"

#include "Application.h"

class MenuItem;

Application* Application::instance = nullptr;

Application::Application() {
    instance = this;

    mainFont = Configuration::getInstance().getValue("Menu.mainFont");
    mainFontSize = Configuration::getInstance().getIntValue("Menu.mainFontSize");
    screenWidth = Configuration::getInstance().getIntValue("Menu.screenWidth");
    screenHeight = Configuration::getInstance().getIntValue("Menu.screenHeight");
    screenDepth = Configuration::getInstance().getIntValue("Menu.screenDepth");

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        // Handle error
        exit(1);
    }

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

    // Set VideoMode
    screen = SDL_SetVideoMode(screenWidth, screenHeight, screenDepth, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (!screen) {
        // Handle error
        exit(1);
    }

    // Initialize TTF fonts
    if (TTF_Init() == -1) {
        exit(1);
    }

    font = TTF_OpenFont(mainFont.c_str(), mainFontSize);
    TTF_SetFontHinting(font, TTF_HINTING_NORMAL);  // or TTF_HINTING_LIGHT, TTF_HINTING_MONO, TTF_HINTING_NONE
    TTF_SetFontKerning(font, 1); // 1 to enable, 0 to disable

    // Create instances for each menu element:
    // * mainMenu contains both the system view (console folders) and system rom browser
    // * systeMenu is the system settings menu
    // * romMenu is the rom settings menu (to select rom specific settings)
    mainMenu = std::make_unique<Menu>();

    createSystemMenu();

    romMenu = std::make_unique<RomMenu>();

    currentState = std::make_unique<State>(mainMenu.get(), systemMenu.get());

    systemMenu->enableSelectionRectangle();
    romMenu->enableSelectionRectangle();

    // Load the list of rom aliases
    SimpleMenuItem::loadAliases();

    // Initialize the menu
    setupMenu();
}

void Application::createSystemMenu() {

    std::string backgroundPath = 
        Configuration::getInstance().getValue("Menu.homePath") 
        + ".simplemenu/resources/settings.png";
    std::string settingsFont = 
        Configuration::getInstance().getValue("Menu.homePath") 
        + ".simplemenu/resources/Akrobat-Bold.ttf";

    this->systemMenu = std::make_unique<SystemMenu>(backgroundPath, settingsFont);

    std::string systemMenuJSON = 
        Configuration::getInstance().getValue("Menu.systemMenuJSON");

    loadMenuFromJSON(systemMenuJSON);
}

ThumbnailCache& Application::getThumbnailCache() {
    return thumbnailCache;
}

Application& Application::getInstance() {
    return *instance;
}

bool Application::fileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

void Application::run() {
    bool isRunning = true;
    SDL_Event event;

    int fps = 0;
    int frameCount = 0;
    Uint32 fpsTimer = 0;

    Uint32 frameStart = 0;

    while (isRunning) {

        int screenRefresh = this->cfg.getIntValue(
                            idToString[SettingId::SCREEN_REFRESH]);

        int frameDelay = 1000 / screenRefresh;

        // Wait if last frame was drawn too fast
        if (SDL_GetTicks() - frameStart < frameDelay) {
            continue;
        }

        // Fine tune FPS
        if (frameCount == screenRefresh 
            and (SDL_GetTicks() - fpsTimer) < 1000) {
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
                    // Render the current menu
                    if (currentState->getCurrentMenu()) {
                        currentState->getCurrentMenu()->render(screen, font, currentState->getCurrentState());
                    }
                    break;
                case SDL_JOYAXISMOTION:
                case SDL_JOYBUTTONDOWN:
                case SDL_JOYBUTTONUP:
                case SDL_JOYHATMOTION:
                    handleJoystickEvents(event);
                    break;
            }
        }

        // Handle FPS information
        if (SDL_GetTicks() - fpsTimer >= 1000) {
            fps = frameCount;
            frameCount = 0;
            fpsTimer = SDL_GetTicks();
        }

        // Clear the screen
        SDL_FillRect(screen, nullptr, SDL_MapRGB(screen->format, 0, 0, 0));

        if (currentState->getCurrentMenu()->getNumberOfItems() == 0) {
            std::cerr << "Error: Current menu has no items!" << std::endl;
            return;  // or handle this error appropriately
        }

        // Then fill the screen and render the menu
        currentState->getCurrentMenu()->render(screen, font, currentState->getCurrentState());

        // TODO add bolean setting to show/hide FPS
        if (this->cfg.getBoolValue(idToString[SettingId::SHOW_FPS])) {
            printFPS(fps);
        }

        SDL_Flip(screen);

        frameCount++;
    
    }
}

void Application::printFPS(int fps) {
    // Display FPS page number / total_pages at the bottom
    std::string fpsText = "FPS: " + std::to_string(fps);
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, fpsText.c_str(), {255,255,0});
    SDL_Rect destRect = {10, 10, 0, 0};  // Positon for page counter
    SDL_BlitSurface(textSurface, NULL, screen, &destRect);
}

State* Application::getCurrentState() const {
    return currentState.get();
}

// Menu* Application::getMainMenu() const {
//     return mainMenu.get();
// }

void Application::showMainMenu() {
    currentState->setCurrentMenu(mainMenu.get());
}

// FIXME
struct ConsoleData {
    std::string name;
    std::vector<std::string> execs;
    std::vector<std::string> romExts;
    std::vector<std::string> romDirs;
};

std::map<std::string, ConsoleData> parseIniFile(const std::string& iniPath) {
    boost::property_tree::ptree pt;
    boost::property_tree::read_ini(iniPath, pt);

    std::map<std::string, ConsoleData> consoleDataMap;
    auto consoleList = pt.get<std::string>("CONSOLES.consoleList");
    std::stringstream ss(consoleList);
    std::string consoleName;

    while (std::getline(ss, consoleName, ',')) {
        ConsoleData data;
        data.name = consoleName;
        
        std::string execs_str = pt.get<std::string>(consoleName + ".execs");
        std::stringstream ss(execs_str);
        std::string exec;
        while (std::getline(ss, exec, ',')) {
            data.execs.push_back(exec);
        }
        std::string romExts_str = pt.get<std::string>(consoleName + ".romExts");
        ss = std::stringstream(romExts_str);
        std::string romExt;
        while (std::getline(ss, romExt, ',')) {
            data.romExts.push_back(romExt);
        }
        std::string romDirs_str = pt.get<std::string>(consoleName + ".romDirs");
        ss = std::stringstream(romDirs_str);
        std::string romDir;
        while (std::getline(ss, romDir, ',')) {
            data.romDirs.push_back(romDir);
        }

        consoleDataMap[consoleName] = data;
    }

    return consoleDataMap;
}


// Set up the main menu items and submenus
// Browse the folders in romsPath, and create menu entries
// Each Menu entry contains a series of MenuItem(s) 
// void Application::setupMenu() {
//     FileManager fileManager;
//     std::string romsPath = Configuration::getInstance().getValue("Menu.romsPath");
//     // Assuming "roms" is the root folder to start from
//     auto folders = fileManager.getFolders(romsPath);

//     for (const auto& folder : folders) {
//         // For each folder, create a MenuItem and populate it with files
//         auto files = fileManager.getFiles(romsPath + folder);
        
//         auto subMenu = std::make_unique<Menu>();
//         subMenu->setParent(mainMenu.get());
//         for (const auto& file : files) {
//             std::string romPath = romsPath + folder + "/" + file;
//             // FIXME MenuItem id should not be ""
//             subMenu->addItem(std::make_unique<SimpleMenuItem>("", file, romPath));
//         }

//         // Add each folder (game system) populated with its own submenu (list of games in the folder)
//         // FIXME MenuItem id should not be ""
//         mainMenu->addItem(std::make_unique<SimpleMenuItem>("", folder, std::move(subMenu)));
//     }

//     std::cout << "Found folders: " << folders.size() << std::endl;
// }

void Application::setupMenu() {
    FileManager fileManager;
    std::string romsPath = Configuration::getInstance().getValue("Menu.romsPath");

    // Load section groups from the section_groups folder
    auto sectionGroups = fileManager.getFiles("/userdata/system/simplermenu_plus/resources/config/x86/.simplemenu/section_groups/");
    
    for (const auto& sectionGroupFile : sectionGroups) {
        std::cout << "SectionGroup: " << sectionGroupFile << std::endl;
        auto sectionMenu = std::make_unique<Menu>();
        // sectionMenu->setParent(sectionMenu.get());
        auto consoleDataMap = parseIniFile("/userdata/system/simplermenu_plus/resources/config/x86/.simplemenu/section_groups/" + sectionGroupFile);
        for (const auto& [key, value] : consoleDataMap) {
            std::cout << "Key: " << key << " RomDir: " << (value.romDirs.empty() ? "EMPTY" : value.romDirs[0]) << std::endl;
        }
        for (const auto& [consoleName, data] : consoleDataMap) {
            std::cout << " - ConsoleName: " << consoleName << std::endl;
            // std::cout << " - data.romDirs: " << data.romDirs[0] << std::endl;

            auto subMenu = std::make_unique<Menu>();
            subMenu->setParent(sectionMenu.get());
            
            for (const auto& romDir : data.romDirs) {
                std::cout << "*** romDir: " << romDir << std::endl;
                auto files = fileManager.getFiles(romDir);
                for (const auto& file : files) {
                    std::string romPath = romDir + file;
                    std::cout << "Adding ROM: " << romPath << std::endl;
                    subMenu->addItem(std::make_unique<SimpleMenuItem>("", file, romPath));
                }
            }
            if (subMenu->getNumberOfItems() > 0) {
                sectionMenu->addItem(std::make_unique<SimpleMenuItem>("", consoleName, std::move(subMenu)));
            }
        }
        if (sectionMenu->getNumberOfItems() >0) {
            mainMenu->addItem(std::make_unique<SimpleMenuItem>("", sectionGroupFile, std::move(sectionMenu)));
        }
    }
}

void Application::handleKeyPress(SDLKey key) {
    switch (key) {
        case SDLK_UP:
            currentState->navigateUp();
            break;
        case SDLK_DOWN:
            currentState->navigateDown();
            break;
        case SDLK_LEFT:
            currentState->navigateLeft();
            break;
        case SDLK_RIGHT:
            currentState->navigateRight();
            break;
        case SDLK_RETURN:
            currentState->enterFolder();
            break;
        case SDLK_SPACE:
            std::cout << "SPACE -> exitFolder" << std::endl;
            currentState->exitFolder();
            break;
        case SDLK_ESCAPE:
            std::cout << "Enter systemSettingsMenu" << std::endl;
            currentState->showSystemMenu();
            break;
        case SDLK_m: 
            std::cout << "rommenu" << std::endl;
            currentState->showRomMenu();
            break;
        default:
            // Do nothing
            break;
        // Handle other keys...
    }
    // Debug, print listings
    //currentState->printCurrentContents();
}

void Application::handleJoystickEvents(SDL_Event& event) {
    switch (event.type) {
        case SDL_JOYAXISMOTION:
            // Handle axis motion events
            // You can map specific axis movements to navigate up, down, left, or right
            break;
        case SDL_JOYBUTTONDOWN:
            // Handle button press events
            // For example, a specific button might be mapped to "enterFolder"
            break;
        case SDL_JOYBUTTONUP:
            // Handle button release events if needed
            break;
        case SDL_JOYHATMOTION:
            // Handle D-pad movements
            break;
        default:
            break;
    }
}

void Application::settingsChanged(const std::string &id, 
                                  const std::string &value) {

    if (id == idToString[SettingId::SCREEN_REFRESH]) {
        this->cfg.setValue(idToString[SettingId::SCREEN_REFRESH], value);

    } else if (id == idToString[SettingId::SHOW_FPS]) {
        this->cfg.setValue(idToString[SettingId::SHOW_FPS], value);
    }   

}

void Application::loadMenuFromJSON(const std::string& jsonPath) {
    // Create a root
    pt::ptree root;

    // Load the JSON file into the property tree
    try {
        pt::read_json(jsonPath, root);
    } catch (const pt::json_parser::json_parser_error& e) {
        std::cerr << "Failed to parse " << jsonPath << ": " << e.what() << std::endl;
        return;
    }

    // Iterate over the menu items
    for (const auto& jsonItem : root.get_child("SystemMenu")) {
        auto type = jsonItem.second.get<std::string>("type");

        std::unique_ptr<SimpleMenuItem> menuItem = nullptr;
        std::string id = jsonItem.second.get<std::string>("id");
        // TODO check if id is known (exists in settingsMap)? 
        std::string title = jsonItem.second.get<std::string>("title");

        if (type == "SimpleMenuItem") {
            menuItem = 
                std::make_unique<SimpleMenuItem>(id, title, "");

        } else if (type == "IntegerMenuItem") {
            std::string value = this->cfg.getValue(id);
            menuItem = 
                std::make_unique<IntegerMenuItem>(id, title, value);
        
        } else if (type == "BooleanMenuItem") {
            std::string value = this->cfg.getValue(id);
            menuItem = 
                std::make_unique<BooleanMenuItem>(id, title, value);

        } else if (type == "MultiOptionMenuItem") {
            std::string value = this->cfg.getValue(id);
            
            std::vector<std::string> options;
            pt::ptree const& children = jsonItem.second.get_child("options");
            for (const auto& child : children) {
                // children is a list of `std::pair("", value)`
                //   - child.first contains the string ""
                //   - child.second contains the option value
                options.push_back(child.second.data());
            }

            menuItem = 
                std::make_unique<MultiOptionMenuItem>(
                    id, title, value, options);
        
        }

        if (menuItem) {
            menuItem->attach(this);
            systemMenu->addItem(std::move(menuItem));
        }

        
    }
}
