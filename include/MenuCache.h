#pragma once
#include <string>
#include <vector>
#include "MenuItem.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace pt = boost::property_tree;

struct CachedMenuItem {
    std::string title;
    std::string path;
};

class MenuCache {
public:
    MenuCache() = default;
    
    // Save the given data to cache
    void saveToCache(const std::string& filePath, const std::vector<CachedMenuItem>& data);

    // Load data from cache
    std::vector<CachedMenuItem> loadFromCache(const std::string& filePath);

    // Check if cache file exists
    bool cacheExists(const std::string& filePath);
};
