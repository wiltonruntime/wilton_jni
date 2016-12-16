/* 
 * File:   wiltonjs_cron.cpp
 * Author: alex
 *
 * Created on September 7, 2016, 1:14 PM
 */

#include "wiltonjs/wiltonjs.hpp"

#include <functional>

#include "staticlib/serialization.hpp"

#include "wilton/wilton.h"

namespace wiltonjs {

namespace { // anonymous

namespace ss = staticlib::serialization;

const std::string EMPTY_STRING = "";

detail::handle_registry<wilton_CronTask>& static_registry() {
    static detail::handle_registry<wilton_CronTask> registry;
    return registry;
}

} // namespace

std::string cron_start(const std::string& data, void* object) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    auto rexpr = std::ref(EMPTY_STRING);
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("expression" == name) {
            rexpr = detail::get_json_string(fi);
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (rexpr.get().empty()) throw WiltonJsException(TRACEMSG(
            "Required parameter 'url' not specified"));
    const std::string& expr = rexpr.get();
    // get runnable
    if (nullptr == object) throw WiltonJsException(TRACEMSG(
            "Required parameter 'runnable' not specified"));
    void* runnable = detail::wrap_object_permanent(object);
    // call wilton
    wilton_CronTask* cron;
    char* err = wilton_CronTask_start(std::addressof(cron), expr.c_str(), expr.length(),
            runnable,
            [](void* runnable_passed) {
                detail::invoke_runnable(runnable_passed);
            });
    if (nullptr != err) detail::throw_wilton_error(err, TRACEMSG(std::string(err)));
    int64_t handle = static_registry().put(cron);
    return ss::dump_json_to_string({
        { "cronHandle", handle}
    });
}

std::string cron_stop(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("cronHandle" == name) {
            handle = detail::get_json_int(fi);
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'httpclientHandle' not specified"));
    // get handle
    wilton_CronTask* cron = static_registry().remove(handle);
    if (nullptr == cron) throw WiltonJsException(TRACEMSG(
            "Invalid 'cronHandle' parameter specified"));
    // call wilton
    char* err = wilton_CronTask_stop(cron);
    if (nullptr != err) {
        static_registry().put(cron);
        detail::throw_wilton_error(err, TRACEMSG(std::string(err)));
    }
    return "{}";
}

} // namespace
