#include "MenuCache.h"
#include <fstream>

void MenuCache::saveToCache(const std::string& filePath, const std::vector<CachedMenuItem>& data) {
    boost::property_tree::ptree root;
    
    for (const auto& item : data) {
        pt::ptree itemNode;
        itemNode.put("section", item.section);
        itemNode.put("folder", item.folder);
        itemNode.put("rom", item.rom);
        itemNode.put("path", item.path);

        root.push_back(std::make_pair(item.rom, itemNode));
    }

    pt::write_json(filePath, root);
    inMemoryCache[filePath] = data;
}

std::vector<CachedMenuItem> MenuCache::loadFromCache(const std::string& filePath) {
    // Check if data is in the in-memory cache
    auto it = inMemoryCache.find(filePath);
    if (it != inMemoryCache.end()) {
        return it->second;
    }

    pt::ptree root;
    pt::read_json(filePath, root);

    std::vector<CachedMenuItem> data;
    for (const auto& kv : root) {
        CachedMenuItem item;
        item.section = kv.second.get<std::string>("section");
        item.folder = kv.second.get<std::string>("folder");
        item.rom = kv.second.get<std::string>("rom");
        item.path = kv.second.get<std::string>("path");

        data.push_back(item);
    }

    // After reading from file, store data in the in-memory cache
    inMemoryCache[filePath] = data;
    
    return data;
}

bool MenuCache::cacheExists(const std::string& filePath) {
    std::ifstream infile(filePath);
    return infile.good();
}
