# SimplerMenu+ Documentation

## Overview

SimplerMenu+ is an emulator launcher written in C++/SDL, based on the SimpleMenu project but built from scratch. The main goal is to provide a feature-rich frontend for emulators similar to EmulationStation while maintaining compatibility with devices that lack GPU acceleration.

## Menu Structure Visualization

```
SimplerMenu+ Menu Hierarchy
│
├── SECTION LEVEL (System categories)
│   │   Configuration: /section_groups/*.ini
│   │   Examples: "consoles.ini", "arcades.ini", "handhelds.ini"
│   │   Access to: SYSTEM SETTINGS (Global settings)
│   │
│   └── FOLDER LEVEL (Specific systems/consoles)
│       │   Configuration: Defined within section_groups/*.ini
│       │   Settings: FOLDER.coreSelection (emulator selection)
│       │   Access to: SYSTEM SETTINGS and FOLDER SETTINGS
│       │
│       └── ROM LEVEL (Individual games)
│           │   Names: Defined in alias.txt, alias_PICO-8.txt
│           │   Thumbnails: Loaded from /media/images/
│           │   Configuration: /rom_preferences/*/[romname]
│           │   Settings: GAME.romOverclock, GAME.romAutostart, GAME.coreOverride
│           │   Access to: SYSTEM SETTINGS, FOLDER SETTINGS and ROM SETTINGS
│
├── SYSTEM SETTINGS (Global configuration)
│   Configuration: config.ini [SYSTEM]
│   Settings: volume, brightness, theme, language, show FPS, etc.
│   Accessible from: Any menu level
│
├── FOLDER SETTINGS (System configuration)
│   Configuration: Stored in memory
│   Settings: Default emulator selection
│   Accessible from: FOLDER and ROM levels
│
└── ROM SETTINGS (Game configuration)
    Configuration: /rom_preferences/*/[romname]
    Settings: Specific overclock, autostart, specific emulator
    Accessible from: ROM level only
```

## Key Features

1. **Multi-level Menu Navigation System**:
   - Section level (Categories of systems)
   - Folder level (Specific systems/consoles)
   - ROM level (Individual games)
   - Settings menus for system, folder, and ROM-specific configurations

2. **Theming Support**:
   - Customizable themes with resolution-specific directories
   - Support for different screen resolutions (320x240, 640x480, 1024x768, 1280x720, etc.)
   - Theme-based layout customization

3. **ROM Management**:
   - Caching system for faster loading
   - File system scanning and indexing
   - Alias system for showing friendly game names instead of filenames

4. **Configuration and Settings**:
   - System-wide settings (volume, brightness, themes, language, FPS display)
   - Folder-specific settings (core/emulator selection)
   - ROM-specific settings (core override, overclock, autostart)

5. **Internationalization**:
   - Multi-language support via i18n.ini
   - Language changeable in runtime

6. **Media Support**:
   - Thumbnail loading for game previews
   - Scrolling text for long titles

## Directory Structure

The application expects the following directory structure:

- `config.ini`: Main configuration file
- `i18n.ini`: Internationalization file
- `.simplemenu/`: Configuration directory containing:
  - `section_groups/`: Configuration files for system categories
  - `themes/`: Theme files organized by resolution
  - `alias.txt`: Game name aliases for various systems
  - `alias_PICO-8.txt`: PICO-8 specific aliases
  - `rom_preferences/`: ROM-specific settings
  - `resources/`: UI resources and images

- `resources/`:
  - `roms/`: Root directory for ROM files, organized by system
  - `bios/`: BIOS files directory
  - `config/`: Additional configuration files

## Implementation Details

### Menu Structure
The menu is organized hierarchically:
1. **Sections**: These are categories of systems (e.g., "consoles", "arcades", "handhelds")
2. **Folders**: These represent specific systems/platforms (e.g., "SNES", "Genesis", "NeoGeo")
3. **ROMs**: Individual game files within each system

### Configuration System
The application uses a flexible configuration system based on INI files:
- Global settings are stored in `config.ini`
- System-specific configurations are defined in section group files
- Each theme has its own configuration file

### Theming System
Themes are stored in resolution-specific folders and include:
- Background images
- Font definitions
- Layout parameters (coordinates, sizes, alignments)
- Color schemes

### ROM Metadata
The system uses an alias system to display friendly names for ROMs instead of filenames:
- `alias.txt` files map ROM filenames to display names
- Thumbnails are loaded from a `media/images/` directory within each ROM folder

### Core/Emulator Selection
Each system can have multiple emulator cores defined:
- Default core selection is configurable at the folder (system) level
- Individual ROMs can override the default core
- Core list is read from the section group configuration files

## Pending Features and TODOs

Several TODOs are scattered throughout the code:

1. Core override functionality for ROMs needs to be fully implemented
2. Dynamic font size adjustment based on screen resolution
3. Fixing clipWidth to use the correct width from theme.ini settings
4. Moving pagination calculation to the constructor for better performance
5. Improving the caching system
6. Additional theme customization options

## Development Environment

The project includes Docker configuration for development:
- `Dockerfile` for containerized development
- VSCode integration with Dev Containers
- Support for debugging and testing on different platforms

## License

The project is licensed under the GNU General Public License v3.0, allowing for free distribution and modification with the requirement that derivative works also be open-source.

## Technical Implementation

The code is organized using object-oriented principles with several key classes:
- `Application`: Main application class handling the program flow
- `Menu`, `Section`, `Folder`, `Rom`: Classes representing the menu structure
- `RenderComponent`: Handles rendering of UI elements
- `Configuration`: Manages application settings
- `Theme`: Handles theme-related settings
- `Settings`: Manages different types of settings (System, Folder, ROM)
- `MenuCache`: Provides caching functionality for faster loading

The application uses SDL for rendering and input handling, making it compatible with a wide range of devices, including those without OpenGL support.