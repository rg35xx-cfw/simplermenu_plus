#pragma once
#include <string>
#include <vector>
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

class Cache {
private:
    std::vector<CachedMenuItem> menuCache;

public:
    Cache() = default;
    
    // Save the given data to cache
    void menuCacheSave(const std::string& filePath, const std::vector<CachedMenuItem>& data);

    // Load data from cache
    std::vector<CachedMenuItem> menuCacheLoad(const std::string& filePath);

    // Update cache data
    bool menuCacheUpdateItem(const std::string& filePath, const std::string& itemPath, const std::string& newCore);

    // Check if cache file exists
    bool menuCacheExists(const std::string& filePath);
};
