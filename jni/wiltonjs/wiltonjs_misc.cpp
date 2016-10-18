/* 
 * File:   wilton_misc.cpp
 * Author: alex
 *
 * Created on October 4, 2016, 8:13 PM
 */

#include "wiltonjs/wiltonjs.hpp"

#include <functional>

#include "wilton/wilton.h"

namespace wiltonjs {

namespace { // anonymous

namespace ss = staticlib::serialization;

const std::string EMPTY_STRING = "";

} // namespace

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

std::string tcp_wait_for_connection(const std::string& data, void*) {
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t timeout = -1;
    auto rip = std::ref(EMPTY_STRING);
    int64_t port = -1;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("ipAddress" == name) {
            rip = detail::get_json_string(fi);
        } else if ("tcpPort" == name) {
            port = detail::get_json_int(fi);
        } else if ("timeoutMillis" == name) {
            timeout = detail::get_json_int(fi);
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == timeout) throw WiltonJsException(TRACEMSG(
            "Required parameter 'timeoutMillis' not specified"));
    if (rip.get().empty()) throw WiltonJsException(TRACEMSG(
            "Required parameter 'ipAddress' not specified"));
    if (-1 == port) throw WiltonJsException(TRACEMSG(
            "Required parameter 'tcpPort' not specified"));
    const std::string& ip = rip.get();
    // call wilton
    char* err = wilton_tcp_wait_for_connection(ip.c_str(), ip.size(),
            static_cast<int> (port), static_cast<int> (timeout));
    if (nullptr != err) {
        detail::throw_wilton_error(err, TRACEMSG(std::string(err)));
    }
    return "{}";
}


} // namespace

