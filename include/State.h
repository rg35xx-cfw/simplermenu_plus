enum MenuLevel {
    MENU_SECTION,
    MENU_FOLDER,
    MENU_ROM,
    SYSTEM_SETTINGS,
    FOLDER_SETTINGS,
    ROM_SETTINGS
};

struct State {
    MenuLevel currentMenuLevel;
    int currentSectionIndex;
    int currentFolderIndex;
    int currentRomIndex;
};