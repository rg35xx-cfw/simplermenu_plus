#pragma once
#include <string>
#include <vector>
#include <map>
// #include "MenuItem.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace pt = boost::property_tree;

struct CachedMenuItem {
    std::string folder;
    std::string rom;
    std::string path;
    std::string core;
};


struct ConsoleData {
    std::string name;
    std::vector<std::string> execs;
    std::vector<std::string> romExts;
    std::vector<std::string> romDirs;
    bool enabled;
    std::string selectedExec;
    std::string category;
    int scaling;
};


class Cache {
private:
    std::vector<CachedMenuItem> menuCache;
    std::map<std::string, ConsoleData> systemsCacheMap;

public:
    Cache() = default;
    
    // Save the given data to cache
    void menuCacheSave(const std::string& filePath, const std::vector<CachedMenuItem>& data);

    // Load data from JSON file
    // If the cache is already loaded in memory, return it
    // Otherwise, load from file and return the data
    std::vector<CachedMenuItem> menuCacheLoad(const std::string& filePath);

    // Update cache data
    bool menuCacheUpdateItem(const std::string& filePath, const std::string& itemPath, const std::string& newCore);

    // Check if cache file exists
    bool menuCacheExists(const std::string& filePath);

    // Load systems from JSON file
    // Retrieve the systems cache (from memory, if was already read, or file
    // if this is the first time we are reading it)
    std::map<std::string, ConsoleData> systemsCacheLoad(const std::string& jsonPath);

    // Update selected exec in systems cache
    bool systemCacheUpdateSelectedExec(const std::string& jsonPath, const std::string& systemName, const std::string& newExec);
};
