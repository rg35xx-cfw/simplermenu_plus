#include "MenuCache.h"
#include <fstream>
#include <iostream>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <cstdio>

void MenuCache::saveToCache(const std::string& filePath, const std::vector<CachedMenuItem>& data) {
    rapidjson::Document doc;
    doc.SetArray();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    for (const auto& item : data) {
        rapidjson::Value obj(rapidjson::kObjectType);
        obj.AddMember("section", rapidjson::Value(item.section.c_str(), allocator), allocator);
        obj.AddMember("folder", rapidjson::Value(item.folder.c_str(), allocator), allocator);
        obj.AddMember("rom", rapidjson::Value(item.rom.c_str(), allocator), allocator);
        obj.AddMember("path", rapidjson::Value(item.path.c_str(), allocator), allocator);
        obj.AddMember("core", rapidjson::Value("default", allocator), allocator);
        doc.PushBack(obj, allocator);
    }

    FILE* fp = fopen(filePath.c_str(), "w");
    char writeBuffer[65536];
    rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
    rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
    doc.Accept(writer);
    fclose(fp);

    inMemoryCache[filePath] = data;
}

std::vector<CachedMenuItem> MenuCache::loadFromCache(const std::string& filePath) {
    auto it = inMemoryCache.find(filePath);
    if (it != inMemoryCache.end()) {
        return it->second;
    }

    std::vector<CachedMenuItem> data;
    FILE* fp = fopen(filePath.c_str(), "r");
    if (!fp) return data;
    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    rapidjson::Document doc;
    doc.ParseStream(is);
    fclose(fp);

    if (doc.IsArray()) {
        for (auto& v : doc.GetArray()) {
            CachedMenuItem item;
            item.section = v["section"].GetString();
            item.folder  = v["folder"].GetString();
            item.rom     = v["rom"].GetString();
            item.path    = v["path"].GetString();
            item.core    = v["core"].GetString();
            data.push_back(item);
        }
    }

    inMemoryCache[filePath] = data;
    return data;
}

bool MenuCache::updateCacheItem(const std::string& filePath, const std::string& itemPath, const std::string& newCore) {
    // Load and update the in-memory cache
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

    // Now update the file using RapidJSON
    rapidjson::Document doc;
    doc.SetArray();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    for (const auto& item : updatedData) {
        rapidjson::Value obj(rapidjson::kObjectType);
        obj.AddMember("section", rapidjson::Value(item.section.c_str(), allocator), allocator);
        obj.AddMember("folder", rapidjson::Value(item.folder.c_str(), allocator), allocator);
        obj.AddMember("rom", rapidjson::Value(item.rom.c_str(), allocator), allocator);
        obj.AddMember("path", rapidjson::Value(item.path.c_str(), allocator), allocator);
        obj.AddMember("core", rapidjson::Value(item.core.c_str(), allocator), allocator);
        doc.PushBack(obj, allocator);
    }

    FILE* fp = fopen(filePath.c_str(), "w");
    if (!fp) {
        return false; // Could not open file for writing
    }
    char writeBuffer[65536];
    rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
    rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
    doc.Accept(writer);
    fclose(fp);

    return true; // Update successful
}

bool MenuCache::cacheExists(const std::string& filePath) {
    std::ifstream infile(filePath);
    return infile.good();
}
