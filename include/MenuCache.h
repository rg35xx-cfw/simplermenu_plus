#pragma once
#include <string>
#include <vector>
// #include "MenuItem.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace pt = boost::property_tree;

struct CachedMenuItem {
    std::string section;
    std::string folder;
    std::string rom;
    std::string path;
    std::string core;
};

class MenuCache {
private:
    std::unordered_map<std::string, std::vector<CachedMenuItem>> inMemoryCache;

public:
    MenuCache() = default;
    
    // Save the given data to cache
    void saveToCache(const std::string& filePath, const std::vector<CachedMenuItem>& data);

    // Load data from cache
    std::vector<CachedMenuItem> loadFromCache(const std::string& filePath);

    // Update cache data
    bool updateCacheItem(const std::string& filePath, const std::string& itemPath, const std::string& newCore, const std::vector<CachedMenuItem>& data);

    // Check if cache file exists
    bool cacheExists(const std::string& filePath);
};
