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

ThumbnailCache& Application::getThumbnailCache() {
    return thumbnailCache;
}

Application& Application::getInstance() {
    return *instance;
}

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

    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    screen = SDL_SetVideoMode(screenWidth, screenHeight, screenDepth, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (!screen) {
        // Handle error
        exit(1);
    }

    if (TTF_Init() == -1) {
        exit(1);
    }

    font = TTF_OpenFont(mainFont.c_str(), mainFontSize);
    TTF_SetFontHinting(font, TTF_HINTING_NORMAL);  // or TTF_HINTING_LIGHT, TTF_HINTING_MONO, TTF_HINTING_NONE
    TTF_SetFontKerning(font, 1); // 1 to enable, 0 to disable

    mainMenu = std::make_unique<Menu>();
    systemMenu = std::make_unique<SystemMenu>();
    currentState = std::make_unique<State>(mainMenu.get());

    // Load the list of rom aliases
    SimpleMenuItem::loadAliases();

    // Set the background for the first folder in the main menu
    if (SubMenuMenuItem* firstFolder = dynamic_cast<SubMenuMenuItem*>(mainMenu->getItem(0))) {
        setBackgroundForFolder(firstFolder->getFolderName());
    }

    std::string folderBackground = Configuration::getInstance().getThemePath() + "resources/general/background.png";
    SDL_Surface* tempSurface = IMG_Load(folderBackground.c_str());
    if (!tempSurface) {
        // Handle the error
        printf("Unable to load image: %s\n", IMG_GetError());
    } else {
        folderBackgroundSurface = SDL_DisplayFormat(tempSurface);
        SDL_FreeSurface(tempSurface);
    }

    // Initialize the menu
    setupMenu();
}

bool Application::fileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

void Application::run() {
    bool isRunning = true;
    SDL_Event event;
    const int FPS = 60;
    const int frameDelay = 1000 / FPS;

    while (isRunning) {
        Uint32 frameStart = SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    isRunning = false;
                    break;
                case SDL_KEYDOWN:
                    handleKeyPress(event.key.keysym.sym);
                    // Clear the screen
                    //SDL_FillRect(screen, nullptr, SDL_MapRGB(screen->format, 0, 0, 0));  // Filling with black
                    // Render the current menu
                    if (currentState->getCurrentMenu()) {
                        currentState->getCurrentMenu()->render(screen, font);
                    }
                    break;
            }
        }

        SDL_FillRect(screen, nullptr, SDL_MapRGB(screen->format, 0, 0, 0));  // Filling with black

        if (currentState->getCurrentMenu() == mainMenu.get() && background) {
            SDL_BlitSurface(background, nullptr, screen, nullptr);
        } else {
            SDL_BlitSurface(folderBackgroundSurface, NULL, screen, NULL);
            currentState->getCurrentMenu()->render(screen, font);
        }

        SDL_Flip(screen);

        Uint32 frameEnd = SDL_GetTicks();
        int frameTime = frameEnd - frameStart;
        if (frameDelay > frameTime) {
            SDL_Delay(frameDelay - frameTime);
        }
    }
}

State* Application::getCurrentState() const {
    return currentState.get();
}

Menu* Application::getMainMenu() const {
    return mainMenu.get();
}

void Application::showSystemMenu() {
    // Display the system menu and handle its input
    // ...

}

void Application::showMainMenu() {
    currentState->setCurrentMenu(mainMenu.get());
}

void Application::setBackgroundForFolder(const std::string& folderName) {
    if (background) {
        SDL_FreeSurface(background);
        background = nullptr;
    }
    std::string backgroundPath = Configuration::getInstance().getThemePath() + "resources/" + folderName + "/logo.png";
    if (fileExists(backgroundPath)) {
        background = IMG_Load(backgroundPath.c_str());
        if (!background) {
            std::cerr << "Failed to load backdrop: " << IMG_GetError() << std::endl;
        }
    } else {
        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
        SDL_Color white = {255, 255, 255};
        SDL_Surface* folderNameSurface = renderText(folderName, white);
        if (folderNameSurface) {
            SDL_Rect dstRect;
            dstRect.x = (screen->w - folderNameSurface->w) / 2;
            dstRect.y = (screen->h - folderNameSurface->h) / 2;
            dstRect.w = folderNameSurface->w;
            dstRect.h = folderNameSurface->h;
            SDL_BlitSurface(folderNameSurface, NULL, screen, &dstRect);
            SDL_FreeSurface(folderNameSurface);
        }
    }
}

SDL_Surface* Application::getBackground() const {
    return background;
}

void Application::clearBackground() {
if (background) {
    SDL_FreeSurface(background);
    background = nullptr;
}
}

SDL_Surface* Application::renderText(const std::string& text, SDL_Color color) {
std::string titleFont = Configuration::getInstance().getValue("Menu.titleFont");
int titleFontSize = Configuration::getInstance().getIntValue("Menu.titleFontSize");
TTF_Font* font = TTF_OpenFont(titleFont.c_str(), titleFontSize);  // Adjust font path and size as necessary
if (!font) {
    // Handle error
    std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
    return nullptr;
}
SDL_Surface* textSurface = TTF_RenderText_Blended(font, text.c_str(), color);
TTF_CloseFont(font);
return textSurface;
}

void Application::setupMenu() {
// Set up the main menu items and submenus
// ...
FileManager fileManager;
std::string romsPath = Configuration::getInstance().getValue("Menu.romsPath");
// Assuming "roms" is the root folder to start from
auto folders = fileManager.getFolders(romsPath);

for (const auto& folder : folders) {
    // For each folder, create a SubMenuMenuItem and populate it with files
    auto files = fileManager.getFiles(romsPath + folder);
    
    auto subMenu = std::make_unique<Menu>();
    subMenu->setParent(mainMenu.get());
    for (const auto& file : files) {
        std::string romPath = romsPath + folder + "/" + file;
        subMenu->addItem(std::make_unique<SimpleMenuItem>(file, romPath));
    }

    mainMenu->addItem(std::make_unique<SubMenuMenuItem>(folder, std::move(subMenu)));
}

//mainMenu->print();
//mainMenu->printContents();
std::cout << "Found folders: " << folders.size() << std::endl;
}

void Application::handleKeyPress(SDLKey key) {
    switch (key) {
        case SDLK_UP:
            currentState->navigateUp();
            if (currentState->getCurrentMenu() == mainMenu.get()) {
                if (SubMenuMenuItem* selectedFolder = dynamic_cast<SubMenuMenuItem*>(currentState->getCurrentMenu()->getItem(currentState->getCurrentMenu()->getSelectedItemIndex()))) {
                    setBackgroundForFolder(selectedFolder->getFolderName());
                }
            }
            break;
        case SDLK_DOWN:
            currentState->navigateDown();
            if (currentState->getCurrentMenu() == mainMenu.get()) {
                if (SubMenuMenuItem* selectedFolder = dynamic_cast<SubMenuMenuItem*>(currentState->getCurrentMenu()->getItem(currentState->getCurrentMenu()->getSelectedItemIndex()))) {
                    setBackgroundForFolder(selectedFolder->getFolderName());
                }
            }
            break;
        case SDLK_LEFT:
            currentState->navigateLeft();
            break;
        case SDLK_RIGHT:
            currentState->navigateRight();

            break;
        case SDLK_RETURN:
            if (dynamic_cast<SubMenuMenuItem*>(currentState->getCurrentMenu()->getItem(currentState->getCurrentMenu()->getSelectedItemIndex()))) {
                if (currentState->getCurrentMenu() == mainMenu.get()) {
                    clearBackground();
                }
                currentState->enterFolder();
            } else {
                currentState->getCurrentMenu()->selectItem();
            }
            break;
        case SDLK_SPACE:
            currentState->exitFolder();
            if (currentState->getCurrentMenu() == mainMenu.get()) {
                if (SubMenuMenuItem* selectedFolder = dynamic_cast<SubMenuMenuItem*>(currentState->getCurrentMenu()->getItem(currentState->getCurrentMenu()->getSelectedItemIndex()))) {
                    setBackgroundForFolder(selectedFolder->getFolderName());
                }
            }
            break;
        case SDLK_ESCAPE:
            std::cout << "systemMenu" << std::endl;
            showSystemMenu();
            break;
        case SDLK_m: 
            std::cout << "rommenu" << std::endl;
            if (currentState->romMenuIsActive()) {
                currentState->hideRomMenu();
            } else {
                currentState->showRomMenu();
            }
            break;
        default:
            // Do nothing
            break;
        // Handle other keys...
    }
    // Debug, print listings
    //currentState->printCurrentContents();
}
