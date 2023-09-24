#pragma once

#include <string>

class HelperUtils {
public:
    std::string getPathWithoutExtension(const std::string& fullPath);
    std::string getFilenameWithoutExtension(const std::string& fullPath);

};
