#include "MenuCache.h"
#include <fstream>
#include <iostream>
#include <boost/locale.hpp>

void MenuCache::saveToCache(const std::string& filePath, const std::vector<CachedMenuItem>& data) {
    boost::property_tree::ptree root;
    boost::locale::generator gen;
    std::locale locale = gen.generate("C.utf-8");

    for (const auto& item : data) {

        pt::ptree itemNode;
        itemNode.put("section", boost::locale::conv::to_utf<char>(item.section, locale));
        itemNode.put("folder", boost::locale::conv::to_utf<char>(item.folder, locale));
        itemNode.put("rom", boost::locale::conv::to_utf<char>(item.rom, locale));
        itemNode.put("path", boost::locale::conv::to_utf<char>(item.path, locale));
        itemNode.put("core", "default");

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
        item.core = kv.second.get<std::string>("core");
        

        data.push_back(item);
    }

    // After reading from file, store data in the in-memory cache
    inMemoryCache[filePath] = data;
    
    return data;
}

bool MenuCache::updateCacheItem(const std::string& filePath, const std::string& itemPath, const std::string& newCore) {
    // Copy of data to modify
    std::vector<CachedMenuItem> updatedData = loadFromCache(filePath);

    // Find and update the item with the specified path
    bool itemFound = false;
    for (auto& item : updatedData) {
        if (item.path == itemPath) {
            item.core = newCore;
            itemFound = true;
            break;
        }
    }

    if (!itemFound) {
        return false; // Item not found
    }

    // Update in-memory cache
    inMemoryCache[filePath] = updatedData;

    // Now update the file
    pt::ptree root;
    for (const auto& item : updatedData) {
        pt::ptree child;
        child.put("section", item.section);
        child.put("folder", item.folder);
        child.put("rom", item.rom);
        child.put("path", item.path);
        child.put("core", item.core);

        root.push_back(std::make_pair("", child));
    }
    pt::write_json(filePath, root);

    return true; // Update successful
}



bool MenuCache::cacheExists(const std::string& filePath) {
    std::ifstream infile(filePath);
    return infile.good();
}
