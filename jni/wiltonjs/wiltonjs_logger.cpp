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

const std::string EMPTY_STRING = "";

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
    auto rlevel = std::ref(EMPTY_STRING);
    auto rlogger = std::ref(EMPTY_STRING);
    auto rmessage = std::ref(EMPTY_STRING);
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("level" == name) {
            rlevel = detail::get_json_string(fi);
        } else if ("logger" == name) {
            rlogger = detail::get_json_string(fi);
        } else if ("message" == name) {
            rmessage = fi.as_string();
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (rlevel.get().empty()) throw WiltonJsException(TRACEMSG(
            "Required parameter 'level' not specified, data: [" + data + "]"));
    if (rlogger.get().empty()) throw WiltonJsException(TRACEMSG(
            "Required parameter 'logger' not specified, data: [" + data + "]"));
    const std::string& level = rlevel.get();
    const std::string& logger = rlogger.get();
    const std::string& message = rmessage.get();
    // call wilton
    char* err = wilton_logger_log(level.c_str(), level.length(), logger.c_str(), logger.length(),
            message.c_str(), message.length());
    if (nullptr != err) detail::throw_wilton_error(err, TRACEMSG(std::string(err) +
            "\nlogger_log error for input data: [" + data + "]"));
    return "{}";
}

std::string logger_is_level_enabled(const std::string& data, void*) {
    // parse json
    ss::JsonValue json = ss::load_json_from_string(data);
    auto rlevel = std::ref(EMPTY_STRING);
    auto rlogger = std::ref(EMPTY_STRING);
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("level" == name) {
            rlevel = detail::get_json_string(fi);
        } else if ("logger" == name) {
            rlogger = detail::get_json_string(fi);
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (rlevel.get().empty()) throw WiltonJsException(TRACEMSG(
            "Required parameter 'level' not specified, data: [" + data + "]"));
    if (rlogger.get().empty()) throw WiltonJsException(TRACEMSG(
            "Required parameter 'logger' not specified, data: [" + data + "]"));
    const std::string& level = rlevel.get();
    const std::string& logger = rlogger.get();
    // call wilton
    int out;
    char* err = wilton_logger_is_level_enabled(logger.c_str(), logger.length(), 
            level.c_str(), level.length(), std::addressof(out));
    if (nullptr != err) detail::throw_wilton_error(err, TRACEMSG(std::string(err) +
            "\nlogger_is_level_enabled error for input data: [" + data + "]"));
    return ss::dump_json_to_string({
        { "enabled", out != 0 }
    });
}

std::string logger_shutdown(const std::string&, void*) {
    // call wilton
    int out;
    char* err = wilton_logger_shutdown();
    if (nullptr != err) detail::throw_wilton_error(err, TRACEMSG(std::string(err) +
            "\nlogger_sutdown error for"));
    return "{}";
}

} // namespace
