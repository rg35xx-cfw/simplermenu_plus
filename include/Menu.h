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

class Section {
private:
    std::string name;
    std::vector<Folder> folders;
public:
    Section(const std::string& name) : name(name) {}

    void addFolder(const Folder& folder) {
        folders.push_back(folder);
    }

    std::string getTitle() const {
        return name;
    }

    Folder* getFolderByName(const std::string& name) {
        for (auto& sys : folders) {
            if (sys.getTitle() == name) {
                return &sys;
            }
        }
        return nullptr;
    }

    const std::vector<Folder>& getFolders() const {
        return folders;
    }

};


class Menu {
private:
    std::vector<Section> sections;
public:
    void addSection(const Section& section) {
        sections.push_back(section);
    }

    Section* getSectionByName(const std::string& name) {
        for (auto& sec : sections) {
            if (sec.getTitle() == name) {
                return &sec;
            }
        }
        return nullptr;
    }

    const std::vector<Section>& getSections() const {
        return sections;
    }
};
