#include "Cache.h"
#include <fstream>
#include <iostream>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <cstdio>


void Cache::menuCacheSave(const std::string& filePath, const std::vector<CachedMenuItem>& data) {
    rapidjson::Document doc;
    doc.SetArray();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    for (const auto& item : data) {
        rapidjson::Value obj(rapidjson::kObjectType);
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

    menuCache = data;
}

std::vector<CachedMenuItem> Cache::menuCacheLoad(const std::string& filePath) {
    
    // Check if the cache is already loaded in memory
    if (menuCache.size() > 0) {
        return menuCache;
    }

    // If not, load from file
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
            item.folder  = v["folder"].GetString();
            item.rom     = v["rom"].GetString();
            item.path    = v["path"].GetString();
            item.core    = v["core"].GetString();
            data.push_back(item);
        }
    }

    menuCache = data;

    return data;
}

bool Cache::menuCacheUpdateItem(const std::string& filePath, const std::string& itemPath, const std::string& newCore) {
    // Load and update the in-memory cache
    std::vector<CachedMenuItem> currentCache = menuCacheLoad(filePath);

    // Find and update the item with the specified path
    bool itemFound = false;
    for (auto& item : currentCache) {
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
    menuCache = currentCache;

    // Now update the file using RapidJSON
    rapidjson::Document doc;
    doc.SetArray();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    for (const auto& item : currentCache) {
        rapidjson::Value obj(rapidjson::kObjectType);
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

bool Cache::menuCacheExists(const std::string& filePath) {
    std::ifstream infile(filePath);
    return infile.good();
}

std::map<std::string, ConsoleData> Cache::systemsCacheLoad(const std::string& jsonPath) {
    
    // Check if the systems cache is already loaded in memory
    if (systemsCacheMap.size() > 0) {
        return systemsCacheMap;
    }

    // If not, load from file
    FILE* fp = fopen(jsonPath.c_str(), "r");
    if (!fp) {
        std::cerr << "Could not open systems file: " << jsonPath << std::endl;
        return systemsCacheMap;
    }

    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    rapidjson::Document doc;
    doc.ParseStream(is);
    fclose(fp);

    if (!doc.IsObject()) {
        std::cerr << "Invalid JSON format in systems file: " << jsonPath << std::endl;
        return systemsCacheMap;
    }

    for (auto it = doc.MemberBegin(); it != doc.MemberEnd(); ++it) {
        const std::string name = it->name.GetString();
        const rapidjson::Value& sys = it->value;

        ConsoleData data;
        data.name = name;

        // execs
        if (sys.HasMember("execs") && sys["execs"].IsArray()) {
            for (const auto& exec : sys["execs"].GetArray()) {
                data.execs.push_back(exec.GetString());
            }
        }

        // romExts
        if (sys.HasMember("romExts") && sys["romExts"].IsArray()) {
            for (const auto& ext : sys["romExts"].GetArray()) {
                data.romExts.push_back(ext.GetString());
            }
        }

        // romDirs
        if (sys.HasMember("romDirs") && sys["romDirs"].IsArray()) {
            for (const auto& dir : sys["romDirs"].GetArray()) {
                data.romDirs.push_back(dir.GetString());
            }
        }

        // selectedExec
        if (sys.HasMember("selectedExec") && sys["selectedExec"].IsString()) {
            data.selectedExec = sys["selectedExec"].GetString();
        }

        // Optionally handle aliasFile, scaling, etc. if needed

        systemsCacheMap[name] = data;
    }

    return systemsCacheMap;
}

bool Cache::systemCacheUpdateSelectedExec(const std::string& jsonPath, 
                                          const std::string& systemName, 
                                          const std::string& newExec) {

    // FIXME: this should be done in memory and then marked to be saved to disk

    FILE* fp = fopen(jsonPath.c_str(), "r");
    if (!fp) return false;

    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    rapidjson::Document doc;
    doc.ParseStream(is);
    fclose(fp);

    if (!doc.IsObject() || !doc.HasMember(systemName.c_str())) return false;

    rapidjson::Value& sys = doc[systemName.c_str()];
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
    sys.RemoveMember("selectedExec");
    sys.AddMember("selectedExec", rapidjson::Value(newExec.c_str(), allocator), allocator);

    fp = fopen(jsonPath.c_str(), "w");
    if (!fp) return false;
    char writeBuffer[65536];
    rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
    rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
    doc.Accept(writer);
    fclose(fp);

    return true;
}