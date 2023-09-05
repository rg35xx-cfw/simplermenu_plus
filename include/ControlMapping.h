#pragma once

#include "Configuration.h"
#include <unordered_map>
#include <string>

class ControlMapping {
private:
    std::unordered_map<std::string, int> controls;

    // Constructor to initialize the mapping from config
    ControlMapping();

public:
    // Static method to get the instance
    static ControlMapping& getInstance();

    // Get the SDL key value for a given control name
    int getControl(const std::string& controlName) const;
};

