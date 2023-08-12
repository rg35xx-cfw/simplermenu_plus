#include "FileManager.h"
#include "Configuration.h"
#include <vector>
#include <string>
#include <set>
#include <filesystem>


std::vector<std::string> FileManager::getFolders(const std::string& path) {
    std::vector<std::string> folders;
    
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_directory()) {
            folders.push_back(entry.path().filename().string());
        }
    }
    
    return folders;
}

// Retrieve a list of files from a given folder
std::vector<std::string> FileManager::getFiles(const std::string& folder) {
    std::vector<std::string> files;
    std::set<std::string> excludedExtensions = Configuration::getInstance().getStringList("Menu.excludedExtensions");

    for (const auto& entry : std::filesystem::directory_iterator(folder)) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            if (excludedExtensions.find(ext) == excludedExtensions.end()) {
                files.push_back(entry.path().filename().string());
            }
        }
    }
    
    return files;
}


