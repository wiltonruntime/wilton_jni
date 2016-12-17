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

// todo: error reporting
bool call_condition(void* cond) {
    try {
        // call condition method
        std::string str = detail::invoke_js_callable(cond, false);
        if (str.empty()) { //exception occured
            return true;
        }
        // json parse
        ss::JsonValue json = ss::load_json_from_string(str);
        int32_t tribool = -1;
        for (const ss::JsonField& fi : json.as_object()) {
            auto& name = fi.name();
            if ("condition" == name) {
                tribool = detail::get_json_bool(fi) ? 1 : 0;
            } else {
                throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
            }
        }
        if (-1 == tribool) throw WiltonJsException(TRACEMSG(
                "Required parameter 'condition' not specified"));
        return 1 == tribool;
    } catch (const std::exception& e) {
        detail::throw_js_exception(TRACEMSG(e.what()));
        // stop waiting on error
        return true;
    }
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
            handle = detail::get_json_int(fi);
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'mutexHandle' not specified"));
    // get handle
    wilton_Mutex* mutex = static_registry().peek(handle);
    if (nullptr == mutex) throw WiltonJsException(TRACEMSG(
            "Invalid 'mutexHandle' parameter specified"));
    // call wilton
    char* err = wilton_Mutex_lock(mutex);
    if (nullptr != err) {
        detail::throw_wilton_error(err, TRACEMSG(std::string(err)));
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
            handle = detail::get_json_int(fi);
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'mutexHandle' not specified"));
    // get handle
    wilton_Mutex* mutex = static_registry().peek(handle);
    if (nullptr == mutex) throw WiltonJsException(TRACEMSG(
            "Invalid 'mutexHandle' parameter specified"));
    // call wilton
    char* err = wilton_Mutex_unlock(mutex);
    if (nullptr != err) {
        detail::throw_wilton_error(err, TRACEMSG(std::string(err)));
    }
    return "{}";
}

std::string mutex_wait(const std::string& data, void* object) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    int64_t timeout_millis = -1;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("mutexHandle" == name) {
            handle = detail::get_json_int(fi);
        } else if ("timeoutMillis" == name) {
            timeout_millis = detail::get_json_int(fi);
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'mutexHandle' not specified"));
    if (-1 == timeout_millis) throw WiltonJsException(TRACEMSG(
            "Required parameter 'timeoutMillis' not specified"));
    // get handle
    wilton_Mutex* mutex = static_registry().peek(handle);
    if (nullptr == mutex) throw WiltonJsException(TRACEMSG(
            "Invalid 'mutexHandle' parameter specified"));
    // call wilton
    char* err = wilton_Mutex_wait(mutex, static_cast<int> (timeout_millis), object,
            [](void* passed) {
                bool cond = call_condition(passed);
                return cond ? 1 : 0;
            });
    if (nullptr != err) {
        detail::throw_wilton_error(err, TRACEMSG(std::string(err)));
    }
    return "{}";
}

std::string mutex_notify_all(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("mutexHandle" == name) {
            handle = detail::get_json_int(fi);
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'mutexHandle' not specified"));
    // get handle
    wilton_Mutex* mutex = static_registry().peek(handle);
    if (nullptr == mutex) throw WiltonJsException(TRACEMSG(
            "Invalid 'mutexHandle' parameter specified"));
    // call wilton
    char* err = wilton_Mutex_notify_all(mutex);
    if (nullptr != err) {
        detail::throw_wilton_error(err, TRACEMSG(std::string(err)));
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
            handle = detail::get_json_int(fi);
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'mutexHandle' not specified"));
    // get handle
    wilton_Mutex* mutex = static_registry().remove(handle);
    if (nullptr == mutex) throw WiltonJsException(TRACEMSG(
            "Invalid 'mutexHandle' parameter specified"));
    // call wilton
    char* err = wilton_Mutex_destroy(mutex);
    if (nullptr != err) {
        static_registry().put(mutex);
        detail::throw_wilton_error(err, TRACEMSG(std::string(err)));
    }
    return "{}";
}

} // namespace

