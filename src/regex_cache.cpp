// ============================================================================
// regex_cache.cpp - Compiled regex pattern cache for performance
// ============================================================================

#include "awk/interpreter.hpp"

namespace awk {

// ============================================================================
// RegexCache Implementation
// ============================================================================

const std::regex& RegexCache::get(const std::string& pattern,
                                   std::regex_constants::syntax_option_type flags) {
    CacheKey key{pattern, flags};

    auto it = cache_.find(key);
    if (it != cache_.end()) {
        ++hits_;
        return *it->second;
    }

    // Cache miss - compile and store
    ++misses_;

    // Shrink cache if needed
    evict_if_needed();

    // Compile regex
    auto regex = std::make_shared<std::regex>(pattern, flags);
    cache_[key] = regex;

    return *regex;
}

void RegexCache::evict_if_needed() {
    // Simple strategy: On overflow, delete half the cache
    // (Real LRU would be more complex to implement)
    if (cache_.size() >= MAX_CACHE_SIZE) {
        // Delete half the entries
        size_t to_remove = cache_.size() / 2;
        auto it = cache_.begin();
        while (to_remove > 0 && it != cache_.end()) {
            it = cache_.erase(it);
            --to_remove;
        }
    }
}

// ============================================================================
// Interpreter - Cached regex access
// ============================================================================

const std::regex& Interpreter::get_cached_regex(const std::string& pattern) {
    return regex_cache_.get(pattern, get_regex_flags());
}

} // namespace awk
