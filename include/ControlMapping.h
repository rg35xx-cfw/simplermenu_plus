#pragma once

#include "Configuration.h"
#include <unordered_map>
#include <string>

class ControlMapping {
private:
    std::unordered_map<std::string, int> controls;
    Configuration cfg;

public:
    // Constructor to initialize the mapping from config
    ControlMapping(Configuration& cfg);

    // Get the SDL key value for a given control name
    int getControl(const std::string& controlName) const;
};

