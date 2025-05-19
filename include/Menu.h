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

class Folder {
private:
    std::string name;
    std::vector<Rom> roms;
public:
    Folder(const std::string& name) : name(name) {}

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
    std::vector<Folder> folders;
public:
    void addFolder(const Folder& folder) {
        folders.push_back(folder);
    }

    Folder* getFolderByName(const std::string& name) {
        for (auto& folder : folders) {
            if (folder.getTitle() == name) {
                return &folder;
            }
        }
        return nullptr;
    }

    const std::vector<Folder>& getFolders() const {
        return folders;
    }
};
