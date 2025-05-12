enum MenuLevel {
    MENU_SECTION,
    MENU_FOLDER,
    MENU_ROM,
    APP_SETTINGS,
    SYSTEM_SETTINGS,
    ROM_SETTINGS
};

struct State {
    MenuLevel currentMenuLevel;
    int currentSectionIndex;
    int currentFolderIndex;
    int currentRomIndex;
    bool launcherCallback;
};