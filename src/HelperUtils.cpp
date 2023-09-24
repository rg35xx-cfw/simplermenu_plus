#include <HelperUtils.h>

std::string HelperUtils::getPathWithoutExtension(const std::string& fullPath) {
    size_t dotPos = fullPath.find_last_of(".");
    size_t slashPos = fullPath.find_last_of("/\\");

    if (dotPos == std::string::npos || (slashPos != std::string::npos && dotPos < slashPos)) {
        // Return the original path if there's no dot after the last slash.
        return fullPath;
    }

    return fullPath.substr(0, dotPos);
}

std::string HelperUtils::getFilenameWithoutExtension(const std::string& fullPath) {
    size_t dotPos = fullPath.find_last_of(".");
    size_t slashPos = fullPath.find_last_of("/\\");

    if (slashPos == std::string::npos) { // No slashes found
        return (dotPos == std::string::npos) ? fullPath : fullPath.substr(0, dotPos);
    }

    std::string filenameWithExtension = fullPath.substr(slashPos + 1);

    if (dotPos == std::string::npos || dotPos < slashPos) { // No dots after the last slash
        return filenameWithExtension;
    }

    return filenameWithExtension.substr(0, filenameWithExtension.find_last_of("."));
}
