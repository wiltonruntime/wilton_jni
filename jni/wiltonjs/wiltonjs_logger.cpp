/* 
 * File:   wiltonjs_logger.cpp
 * Author: alex
 *
 * Created on August 21, 2016, 1:14 PM
 */

#include "wiltonjs/wiltonjs.hpp"

#include <functional>

#include "staticlib/serialization.hpp"

#include "wilton/wilton.h"

namespace wiltonjs {

namespace { // anonymous

namespace ss = staticlib::serialization;

} // namespace

std::string logger_initialize(const std::string& data, void*) {
    char* err = wilton_logger_initialize(data.c_str(), data.length());
    if (nullptr != err) detail::throw_wilton_error(err, TRACEMSG(std::string(err) +
            "\nlogger_initialize error for input data: [" + data + "]"));
    return "{}";
}

std::string logger_log(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    const std::string& level = json.get("level").get_string();
    const std::string& logger = json.get("logger").get_string();
    const std::string& message = json.get("message").get_string();
    // call wilton
    char* err = wilton_logger_log(level.c_str(), level.length(), logger.c_str(), logger.length(),
            message.c_str(), message.length());
    if (nullptr != err) detail::throw_wilton_error(err, TRACEMSG(std::string(err) +
            "\nlogger_log error for input data: [" + data + "]"));
    return "{}";
}

std::string logger_is_level_enabled(const std::string& data, void*) {
    ss::JsonValue json = ss::load_json_from_string(data);
    const std::string& level = json.get("level").get_string();
    const std::string& logger = json.get("logger").get_string();
    int out;
    char* err = wilton_logger_is_level_enabled(logger.c_str(), logger.length(), 
            level.c_str(), level.length(), std::addressof(out));
    if (nullptr != err) detail::throw_wilton_error(err, TRACEMSG(std::string(err) +
            "\nlogger_is_level_enabled error for input data: [" + data + "]"));
    return ss::dump_json_to_string({
        { "enabled", out != 0 }
    });
}

} // namespace
