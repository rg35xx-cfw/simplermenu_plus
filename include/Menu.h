#pragma once
#include <string>
#include <vector>

class Rom {
private:
    std::string name;
    std::string path;
public:
    Rom(const std::string& name, const std::string& path) 
        : name(name), path(path) {}

    std::string getTitle() const {
        return name;
    }

    std::string getPath() const {
        return path;
    }

};

class System {
private:
    std::string name;
    std::vector<Rom> roms;
public:
    System(const std::string& name) : name(name) {}

    void addRom(const Rom& rom) {
        roms.push_back(rom);
    }

    std::string getTitle() const {
        return name;
    }

    const std::vector<Rom>& getRoms() const {
        return roms;
    }

};

class Menu {
private:
    std::vector<System> systems;
public:
    void addSystem(const System& system) {
        systems.push_back(system);
    }

    System* getSystemByName(const std::string& name) {
        for (auto& system : systems) {
            if (system.getTitle() == name) {
                return &system;
            }
        }
        return nullptr;
    }

    const std::vector<System>& getSystems() const {
        return systems;
    }
};
