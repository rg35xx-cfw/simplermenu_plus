#include <memory>
#include <string>
#include <fstream>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

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

    this->intSettings[this->SCREEN_REFRESH] = 
        Configuration::getInstance().getIntValue("Menu." + this->SCREEN_REFRESH); 

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
    systemMenu = std::make_unique<SystemMenu>();
    romMenu = std::make_unique<RomMenu>();

    currentState = std::make_unique<State>(mainMenu.get());

    systemMenu->enableSelectionRectangle();
    romMenu->enableSelectionRectangle();

    // Load the list of rom aliases
    SimpleMenuItem::loadAliases();

    // Initialize the menu
    setupMenu();
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

        int frameDelay = 1000 / this->intSettings[this->SCREEN_REFRESH];

        // Wait if last frame was drawn too fast
        if (SDL_GetTicks() - frameStart < frameDelay) {
            continue;
        }

        // Fine tune FPS
        if (frameCount == this->intSettings[this->SCREEN_REFRESH] 
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

        // First, blit the background if one exists
        // int selectedIndex = currentState->getCurrentMenu()->getSelectedItemIndex();
        // MenuItem* selectedItem = currentState->getCurrentMenu()->getItem(selectedIndex);
        // if (!selectedItem) {
        //     std::cerr << "Error: Selected item is null!" << std::endl;
        //     return;  // or handle this error appropriately
        // }
        MenuItem* selectedItem = currentState->getCurrentMenu()->getItem(currentState->getCurrentMenu()->getSelectedItemIndex());
        if (selectedItem->getBackground()) {
            SDL_BlitSurface(selectedItem->getBackground(), NULL, screen, NULL);
        } else {
            if (currentState->getCurrentMenu() == mainMenu.get()) {
                MenuItem* selectedItem = currentState->getCurrentMenu()->getItem(currentState->getCurrentMenu()->getSelectedItemIndex());
                selectedItem->determineAndSetBackground(screen);
            }
        }

        // Then fill the screen and render the menu
        currentState->getCurrentMenu()->render(screen, font, currentState->getCurrentState());

        printFPS(fps);

        SDL_Flip(screen);

        frameCount++;
         
        // int frameTime = SDL_GetTicks() - frameStart;
        // if (frameDelay > frameTime) {
        //     SDL_Delay(frameDelay - frameTime);
        // }
    
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

// Set up the main menu items and submenus
// Browse the folders in romsPath, and create menu entries
// Each Menu entry contains a series of MenuItem(s) 
void Application::setupMenu() {
    FileManager fileManager;
    std::string romsPath = Configuration::getInstance().getValue("Menu.romsPath");
    // Assuming "roms" is the root folder to start from
    auto folders = fileManager.getFolders(romsPath);

    for (const auto& folder : folders) {
        // For each folder, create a MenuItem and populate it with files
        auto files = fileManager.getFiles(romsPath + folder);
        
        auto subMenu = std::make_unique<Menu>();
        subMenu->setParent(mainMenu.get());
        for (const auto& file : files) {
            std::string romPath = romsPath + folder + "/" + file;
            subMenu->addItem(std::make_unique<SimpleMenuItem>(file, romPath));
        }

        // Add each folder (game system) populated with its own submenu (list of games in the folder)
        mainMenu->addItem(std::make_unique<SimpleMenuItem>(folder, std::move(subMenu)));
    }

    std::cout << "Found folders: " << folders.size() << std::endl;
}

void Application::handleKeyPress(SDLKey key) {
    switch (key) {
        case SDLK_UP:
            currentState->navigateUp();
            break;
        case SDLK_DOWN:
            currentState->navigateDown();
            break;
        case SDLK_LEFT: {
            MenuItem* selectedItem = currentState->navigateLeft();
            if (selectedItem->getTitle() == this->SCREEN_REFRESH) {
                std::cout << "screenRefresh" << std::endl;
                this->intSettings[this->SCREEN_REFRESH] = std::stoi(selectedItem->getValue());
            }
            break;
        }
        case SDLK_RIGHT:{
            MenuItem* selectedItem = currentState->navigateRight();
            if (selectedItem->getTitle() == this->SCREEN_REFRESH) {
                std::cout << "screenRefresh" << std::endl;
                this->intSettings[this->SCREEN_REFRESH] = std::stoi(selectedItem->getValue());
            }
            break;
        }
        case SDLK_RETURN:
            currentState->enterFolder();
            break;
        case SDLK_SPACE:
            std::cout << "SPACE -> exitFolder" << std::endl;
            currentState->exitFolder();
            break;
        case SDLK_ESCAPE:
            std::cout << "systemMenu" << std::endl;
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
