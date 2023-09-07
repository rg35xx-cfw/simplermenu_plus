#include <memory>
#include <string>
#include <fstream>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "Configuration.h"
#include "Theme.h"
#include "Menu.h"
#include "MenuItem.h"
#include "ThumbnailCache.h"
#include "State.h"

#include "Application.h"

class MenuItem;

Application* Application::instance = nullptr;

Application::Application() : controlMapping(ControlMapping::getInstance()) {
    instance = this;

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
    screen = SDL_SetVideoMode(
        this->cfg.getIntValue(SettingId::SCREEN_WIDTH), 
        this->cfg.getIntValue(SettingId::SCREEN_HEIGHT),
        this->cfg.getIntValue(SettingId::SCREEN_DEPTH), 
        SDL_HWSURFACE | SDL_DOUBLEBUF);
    
    if (!screen) {
        // Handle error
        exit(1);
    }

    // Initialize TTF fonts
    if (TTF_Init() == -1) {
        exit(1);
    }

    font = TTF_OpenFont(
    theme.getValue("GENERAL.font", true).c_str(), 
    theme.getIntValue("GENERAL.font_size"));
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
        this->cfg.getValue(SettingId::HOME_PATH) 
        + ".simplemenu/resources/settings.png";
    std::string settingsFont = 
        this->cfg.getValue(SettingId::HOME_PATH) 
        + ".simplemenu/resources/Akrobat-Bold.ttf";

    this->systemMenu = std::make_unique<SystemMenu>(backgroundPath, settingsFont);

    std::string systemMenuJSON = 
        this->cfg.getValue(SettingId::SYSTEM_MENU_JSON);

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

    Uint32 currentTime = SDL_GetTicks();
    nextRepeatTime = currentTime + initialDelay;

    while (isRunning) {

        // Handle button repetition outside of the event processing loop
        currentTime = SDL_GetTicks();
        //std::cout << buttonPressed << " ... " << currentTime << " ... " << nextRepeatTime <<std::endl;
        if (buttonPressed != -1 && currentTime >= nextRepeatTime) {
            // Simulate button press for repetition
            SDL_Event repeatEvent;
            repeatEvent.type = SDL_JOYBUTTONDOWN;
            repeatEvent.jbutton.button = buttonPressed;
            SDL_PushEvent(&repeatEvent);
            nextRepeatTime += repeatInterval;
        }

        int screenRefresh = this->cfg.getIntValue(SettingId::SCREEN_REFRESH);

        int frameDelay = 1000 / screenRefresh;

        // Wait if last frame was drawn too fast
        if (SDL_GetTicks() - frameStart < frameDelay) {
            continue;
        }

        // Fine tune FPS
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

        if (currentState->getCurrentMenu()->getNumberOfItems() == 0) {
            std::cerr << "Error: Current menu has no items!" << std::endl;
            return;  // or handle this error appropriately
        }

        // Then fill the screen and render the menu
        currentState->getCurrentMenu()->render(screen, font, currentState->getCurrentState());

        // TODO add bolean setting to show/hide FPS
        if (this->cfg.getBoolValue(SettingId::SHOW_FPS)) {
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

void Application::showMainMenu() {
    currentState->setCurrentMenu(mainMenu.get());
}

void Application::setupMenu() {
    FileManager fileManager;

    // Load section groups from the section_groups folder
    auto sectionGroups = fileManager.getFiles("/userdata/system/.simplemenu/section_groups/");
    
    for (const auto& sectionGroupFile : sectionGroups) {
        auto sectionMenu = std::make_unique<Menu>(sectionGroupFile);
        auto consoleDataMap = Configuration::getInstance().parseIniFile("/userdata/system/.simplemenu/section_groups/" + sectionGroupFile);
        for (const auto& [consoleName, data] : consoleDataMap) {
            auto subMenu = std::make_unique<Menu>(consoleName);
            subMenu->setParent(sectionMenu.get());
            subMenu->setRootMenu(sectionMenu.get());
            
            for (const auto& romDir : data.romDirs) {
                auto files = fileManager.getFiles(romDir);
                for (const auto& file : files) {
                    std::string romPath = romDir + file;
                    auto simpleMenuItem = std::make_unique<SimpleMenuItem>(SettingId::None, file, romPath);
                    simpleMenuItem->setParentMenu(subMenu.get());  // Setting parent for SimpleMenuItem
                    simpleMenuItem->setRootMenu(sectionMenu.get());  // Setting root for SimpleMenuItem
                    subMenu->addItem(std::move(simpleMenuItem));
                }
            }
            if (subMenu->getNumberOfItems() > 0) {
                sectionMenu->addItem(std::make_unique<SimpleMenuItem>(SettingId::None, consoleName, std::move(subMenu), subMenu->getNumberOfItems()));
            }
        }
        if (sectionMenu->getNumberOfItems() >0) {
            mainMenu->addItem(std::make_unique<SimpleMenuItem>(SettingId::None, sectionGroupFile, std::move(sectionMenu)));
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
    static Uint32 lastButtonPressTime = 0;

    switch (event.type) {
        case SDL_JOYAXISMOTION:
            // Handle axis motion events
            // You can map specific axis movements to navigate up, down, left, or right
            break;
        case SDL_JOYBUTTONDOWN:
        {
            if (buttonPressed != event.jbutton.button) {
                buttonPressed = event.jbutton.button;
                lastButtonPressTime = SDL_GetTicks();
                repeatStartTime = lastButtonPressTime + initialDelay;
                nextRepeatTime = lastButtonPressTime + initialDelay;
            }
            // Handle button press events
            //int buttonPressed = event.jbutton.button; 
            if (buttonPressed == controlMapping.getControl("A")) {
                currentState->enterFolder();
            } else if (buttonPressed == controlMapping.getControl("B")) {
                currentState->exitFolder();
            } else if (buttonPressed == controlMapping.getControl("UP")) {
                currentState->navigateUp();
            } else if (buttonPressed == controlMapping.getControl("DOWN")) {
                currentState->navigateDown();
            } else if (buttonPressed == controlMapping.getControl("LEFT")) {
                currentState->navigateLeft();
            } else if (buttonPressed == controlMapping.getControl("RIGHT")) {
                currentState->navigateRight();
            } else if (buttonPressed == controlMapping.getControl("START")) {
                currentState->showSystemMenu();
            } else if (buttonPressed == controlMapping.getControl("SELECT")) {
                currentState->showRomMenu();
            }                    
            break;
        }
        case SDL_JOYBUTTONUP:
            // Handle button release events if needed
            if (event.jbutton.button == buttonPressed) {
                buttonPressed = -1;
                lastButtonPressTime = 0;
            }
            break;
        case SDL_JOYHATMOTION:
            // Handle D-pad movements
            break;
        default:
            break;
    }
}

void Application::settingsChanged(const SettingId &id, 
                                  const std::string &value) {
    switch (id) {
        case SettingId::SCREEN_REFRESH:
            this->cfg.setValue(SettingId::SCREEN_REFRESH, value);
            break;
        
        case SettingId::SHOW_FPS:
            this->cfg.setValue(SettingId::SHOW_FPS, value);
            break;
        default:
            break;
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
        SettingId id = cfg.getSettingId(
            jsonItem.second.get<std::string>("id"));
        // TODO what if id is unknown? 
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
