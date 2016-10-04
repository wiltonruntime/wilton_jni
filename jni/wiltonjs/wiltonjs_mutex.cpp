/* 
 * File:   wiltonjs_mutex.cpp
 * Author: alex
 *
 * Created on October 4, 2016, 7:56 PM
 */

#include "wiltonjs/wiltonjs.hpp"

#include "wilton/wilton.h"

namespace wiltonjs {

namespace { // anonymous

namespace ss = staticlib::serialization;

detail::handle_registry<wilton_Mutex>& static_registry() {
    static detail::handle_registry<wilton_Mutex> registry;
    return registry;
}

} // namespace

std::string mutex_create(const std::string&, void*) {
    wilton_Mutex* mutex;
    char* err = wilton_Mutex_create(std::addressof(mutex));
    if (nullptr != err) detail::throw_wilton_error(err, TRACEMSG(std::string(err) +
            "\nmutex_create error"));
    int64_t handle = static_registry().put(mutex);
    return ss::dump_json_to_string({
        { "mutexHandle", handle}
    });
}

std::string mutex_lock(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("mutexHandle" == name) {
            handle = detail::get_json_handle(fi);
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'mutexHandle' not specified, data: [" + data + "]"));
    // get handle
    wilton_Mutex* mutex = static_registry().remove(handle);
    if (nullptr == mutex) throw WiltonJsException(TRACEMSG(
            "Invalid 'mutexHandle' parameter specified: [" + data + "]"));
    // call wilton
    char* err = wilton_Mutex_lock(mutex);
    static_registry().put(mutex);
    if (nullptr != err) {
        detail::throw_wilton_error(err, TRACEMSG(std::string(err) +
                "\nmutex_lock error for input data: [" + data + "]"));
    }
    return "{}";
}

std::string mutex_unlock(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("mutexHandle" == name) {
            handle = detail::get_json_handle(fi);
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'mutexHandle' not specified, data: [" + data + "]"));
    // get handle
    wilton_Mutex* mutex = static_registry().remove(handle);
    if (nullptr == mutex) throw WiltonJsException(TRACEMSG(
            "Invalid 'mutexHandle' parameter specified: [" + data + "]"));
    // call wilton
    char* err = wilton_Mutex_unlock(mutex);
    static_registry().put(mutex);
    if (nullptr != err) {
        detail::throw_wilton_error(err, TRACEMSG(std::string(err) +
                "\nmutex_unlock error for input data: [" + data + "]"));
    }
    return "{}";
}

std::string mutex_destroy(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("mutexHandle" == name) {
            handle = detail::get_json_handle(fi);
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'mutexHandle' not specified, data: [" + data + "]"));
    // get handle
    wilton_Mutex* mutex = static_registry().remove(handle);
    if (nullptr == mutex) throw WiltonJsException(TRACEMSG(
            "Invalid 'mutexHandle' parameter specified: [" + data + "]"));
    // call wilton
    char* err = wilton_Mutex_destroy(mutex);
    if (nullptr != err) {
        static_registry().put(mutex);
        detail::throw_wilton_error(err, TRACEMSG(std::string(err) +
                "\nmutex_destroy error for input data: [" + data + "]"));
    }
    return "{}";
}

} // namespace

