/* 
 * File:   wiltonjs_shared.cpp
 * Author: alex
 *
 * Created on December 9, 2016, 9:18 PM
 */

#include "wiltonjs/wiltonjs.hpp"

#include "wilton/wilton.h"

namespace wiltonjs {

namespace { // anonymous

namespace ss = staticlib::serialization;

const std::string EMPTY_STRING = "";

} // namespace

std::string shared_put(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    auto rkey = std::ref(EMPTY_STRING);
    auto rvalue = std::ref(EMPTY_STRING);
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("key" == name) {
            rkey = detail::get_json_string(fi);
        } else if ("value" == name) {
            rvalue = detail::get_json_string(fi);
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (rkey.get().empty()) throw WiltonJsException(TRACEMSG(
            "Required parameter 'key' not specified"));
    const std::string& key = rkey.get();
    if (rvalue.get().empty()) throw WiltonJsException(TRACEMSG(
            "Required parameter 'value' not specified"));
    const std::string& value = rvalue.get();
    // call wilton
    char* err = wilton_shared_put(key.c_str(), key.length(), value.c_str(), value.length());
    if (nullptr != err) {
        detail::throw_wilton_error(err, TRACEMSG(std::string(err)));
    }
    return "{}";

}

std::string shared_get(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    auto rkey = std::ref(EMPTY_STRING);
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("key" == name) {
            rkey = detail::get_json_string(fi);
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (rkey.get().empty()) throw WiltonJsException(TRACEMSG(
            "Required parameter 'key' not specified"));
    const std::string& key = rkey.get();
    // call wilton
    char* out;
    int out_len;
    char* err = wilton_shared_get(key.c_str(), key.length(), 
            std::addressof(out), std::addressof(out_len));
    if (nullptr != err) {
        detail::throw_wilton_error(err, TRACEMSG(std::string(err)));
    }
    if (nullptr == out) {
        return ""; // invalid json, should be checked by caller
    }
    return detail::wrap_wilton_output(out, out_len);
}

std::string shared_wait_change(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t timeout_millis = -1;
    auto rkey = std::ref(EMPTY_STRING);
    auto rcvalue = std::ref(EMPTY_STRING);
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("timeoutMillis" == name) {
            timeout_millis = detail::get_json_int(fi);
        } else if ("key" == name) {
            rkey = detail::get_json_string(fi);
        } else if ("currentValue" == name) {
            rcvalue = detail::get_json_string(fi);
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == timeout_millis) throw WiltonJsException(TRACEMSG(
            "Required parameter 'timeoutMillis' not specified"));
    if (rkey.get().empty()) throw WiltonJsException(TRACEMSG(
            "Required parameter 'key' not specified"));
    const std::string& key = rkey.get();
    if (rcvalue.get().empty()) throw WiltonJsException(TRACEMSG(
            "Required parameter 'currentValue' not specified"));
    const std::string& cvalue = rcvalue.get();
    // call wilton
    char* out;
    int out_len;
    char* err = wilton_shared_wait_change(static_cast<int> (timeout_millis),
            key.c_str(), key.length(), cvalue.c_str(), cvalue.length(),
            std::addressof(out), std::addressof(out_len));
    if (nullptr != err) {
        detail::throw_wilton_error(err, TRACEMSG(std::string(err)));
    }
    if (nullptr == out) {
        return ""; // invalid json, should be checked by caller
    }
    return detail::wrap_wilton_output(out, out_len);
}

std::string shared_remove(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    auto rkey = std::ref(EMPTY_STRING);
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("key" == name) {
            rkey = detail::get_json_string(fi);
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (rkey.get().empty()) throw WiltonJsException(TRACEMSG(
            "Required parameter 'key' not specified"));
    const std::string& key = rkey.get();
    // call wilton
    char* err = wilton_shared_remove(key.c_str(), key.length());
    if (nullptr != err) {
        detail::throw_wilton_error(err, TRACEMSG(std::string(err)));
    }
    return "{}";
}

} // namespace
