/* 
 * File:   wiltoncall.cpp
 * Author: alex
 * 
 * Created on August 21, 2016, 8:08 PM
 */

#include "wiltonjs/wiltoncall.hpp"

#include <mutex>
#include <unordered_map>

#include "staticlib/serialization.hpp"

namespace wiltonjs {

namespace { // anonymous

namespace ss = staticlib::serialization;

using fun_type = std::function<std::string(const std::string&, void*)>;
using map_type = std::unordered_map<std::string, fun_type>;

map_type& static_map() {
    static map_type map{};
    return map;
}

std::mutex& static_mutex() {
    static std::mutex mutex{};
    return mutex;
}

} // namespace

void put_wilton_function(const std::string& name, fun_type fun) {
    std::lock_guard<std::mutex> guard{static_mutex()};
    if (name.empty()) {
        throw WiltonJsException(TRACEMSG("Invalid empty wilton_function name specified"));
    }
    auto pa = static_map().emplace(name, fun);
    if (!pa.second) {
        throw WiltonJsException(TRACEMSG("Invalid duplicate wilton_function name specified: [" + name + "]"));
    }
}

std::string invoke_wilton_function(const std::string& name, const std::string& data, void* object) {
    if (name.empty()) {
        throw WiltonJsException(TRACEMSG("Invalid empty wilton_function name specified"));
    }
    try {
        // get function
        fun_type fun = [](const std::string&, void*) {
            return std::string();
        };
        {
            std::lock_guard<std::mutex> guard{static_mutex()};
            auto it = static_map().find(name);
            if (static_map().end() == it) {
                throw WiltonJsException(TRACEMSG(
                        "Invalid unknown wilton_function name specified: [" + name + "]"));
            }
            fun = it->second;
        }
        // invoke
        return fun(data, object);
    } catch (const std::exception& e) {
        throw WiltonJsException(TRACEMSG(e.what() + "\nwiltoncall error for function: [" + name + "]"));
    }
}

} // namespace

