/* 
 * File:   wiltonjs_thread.cpp
 * Author: alex
 *
 * Created on December 9, 2016, 10:31 PM
 */

#include "wiltonjs/wiltonjs.hpp"

#include <functional>

#include "wilton/wilton.h"

namespace wiltonjs {

namespace { // anonymous

namespace ss = staticlib::serialization;

const std::string EMPTY_STRING = "";

} // namespace

// todo: delete
std::string thread_run(const std::string&, void* object) {
    void* runnable = detail::wrap_object_permanent(object);
    // call wilton
    char* err = wilton_thread_run(runnable,
            [](void* runnable_passed) {
                detail::invoke_callable(runnable_passed);
                // env->DeleteGlobalRef(runnable_passed);
            });
    if (nullptr != err) {
        detail::throw_wilton_error(err, TRACEMSG(std::string(err)));
    }
    return "{}";
}

std::string thread_sleep_millis(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t millis = -1;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("millis" == name) {
            millis = detail::get_json_int(fi);
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == millis) throw WiltonJsException(TRACEMSG(
            "Required parameter 'millis' not specified"));
    // call wilton
    char* err = wilton_thread_sleep_millis(static_cast<int> (millis));
    if (nullptr != err) {
        detail::throw_wilton_error(err, TRACEMSG(std::string(err)));
    }
    return "{}";
}

} // namespace



