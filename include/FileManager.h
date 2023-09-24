#pragma once
#include <vector>
#include <string>

#include "Configuration.h"

class FileManager {

private:
    Configuration cfg;

public:

    FileManager(Configuration& cfg): cfg(cfg) {};

    std::vector<std::string> getFolders(const std::string& path);
    std::vector<std::string> getFiles(const std::string& folder);

};

