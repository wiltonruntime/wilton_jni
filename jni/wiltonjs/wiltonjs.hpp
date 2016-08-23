/* 
 * File:   wiltonjs.hpp
 * Author: alex
 *
 * Created on August 21, 2016, 1:15 PM
 */

#ifndef WILTONJS_WILTONJS_HPP
#define	WILTONJS_WILTONJS_HPP

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_set>

#include "wiltonjs/WiltonJsException.hpp"

namespace wiltonjs {

// Mustache

std::string mustache_render(const std::string& data, void* object);

// HttpClient

std::string httpclient_create(const std::string& data, void* object);

std::string httpclient_close(const std::string& data, void* object);

std::string httpclient_execute(const std::string& data, void* object);

std::string httpclient_send_temp_file(const std::string& data, void* object);


// internal

namespace detail {

void throw_wilton_error(char* err, const std::string& msg);

std::string wrap_wilton_output(char* out, int out_len);


template<typename T>
class handle_registry {
    std::unordered_set<T*> registry;
    std::mutex mutex;

public:
    int64_t put(T* ptr) {
        std::lock_guard<std::mutex> lock(mutex);
        auto pair = registry.insert(ptr);
        return pair.second ? reinterpret_cast<int64_t> (ptr) : 0;
    }

    T* remove(int64_t handle) {
        std::lock_guard<std::mutex> lock(mutex);
        T* ptr = reinterpret_cast<T*> (handle);
        auto erased = registry.erase(ptr);
        return 1 == erased ? ptr : nullptr;
    }
};

} // namespace

} // namespace

#endif	/* WILTONJS_WILTONJS_HPP */

