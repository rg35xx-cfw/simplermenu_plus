#include "MenuCache.h"
#include <fstream>

void MenuCache::saveToCache(const std::string& filePath, const std::vector<CachedMenuItem>& data) {
    boost::property_tree::ptree root;
    
    for (const auto& item : data) {
        pt::ptree itemNode;
        itemNode.put("title", item.title);
        itemNode.put("path", item.path);

        root.push_back(std::make_pair(item.title, itemNode));
    }

    pt::write_json(filePath, root);
}

std::vector<CachedMenuItem> MenuCache::loadFromCache(const std::string& filePath) {
    pt::ptree root;
    pt::read_json(filePath, root);

    std::vector<CachedMenuItem> data;
    for (const auto& kv : root) {
        CachedMenuItem item;
        item.title = kv.second.get<std::string>("title");
        item.path = kv.second.get<std::string>("path");

        data.push_back(item);
    }

    return data;
}

bool MenuCache::cacheExists(const std::string& filePath) {
    std::ifstream infile(filePath);
    return infile.good();
}
