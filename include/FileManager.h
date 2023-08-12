#pragma once
#include <vector>
#include <string>

class FileManager {
public:

    std::vector<std::string> getFolders(const std::string& path);
    std::vector<std::string> getFiles(const std::string& folder);

};

