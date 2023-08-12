#pragma once
#include <unordered_map>
#include <string>

#include <SDL/SDL.h>

class ThumbnailCache {
private:
    std::unordered_map<std::string, SDL_Surface*> cache;
    const size_t MAX_SIZE = 1000;  // adjust this based on your needs

public:
    SDL_Surface* get(const std::string& key);

    void set(const std::string& key, SDL_Surface* surface);

    ~ThumbnailCache() {
        for (auto& pair : cache) {
            SDL_FreeSurface(pair.second);
        }
    }
};
