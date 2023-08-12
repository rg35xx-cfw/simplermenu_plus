#include "ThumbnailCache.h"
#include <map>

SDL_Surface* ThumbnailCache::get(const std::string& key) {
    if (cache.find(key) != cache.end()) {
        return cache[key];
    }
    return nullptr;
}

void ThumbnailCache::set(const std::string& key, SDL_Surface* surface) {
    if (cache.size() >= MAX_SIZE) {
        // Evict the first (or any other policy) item for simplicity
        if (cache.begin()->second != surface) {
            SDL_FreeSurface(cache.begin()->second);
        }           
        cache.erase(cache.begin());
    }
    cache[key] = surface;
}

