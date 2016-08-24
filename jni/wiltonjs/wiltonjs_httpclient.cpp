/* 
 * File:   wiltonjs_httpclient.cpp
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

detail::handle_registry<wilton_HttpClient>& static_registry() {
    static detail::handle_registry<wilton_HttpClient> registry;
    return registry;
}

} // namespace

std::string httpclient_create(const std::string& data, void*) {
    wilton_HttpClient* http;
    char* err = wilton_HttpClient_create(std::addressof(http), data.c_str(), data.length());
    if (nullptr != err) detail::throw_wilton_error(err, TRACEMSG(std::string(err) +
            "\nhttpclient_create error for input data: [" + data + "]"));
    int64_t handle = static_registry().put(http);
    return ss::dump_json_to_string({
        { "httpclientHandle", handle }
    });
}

std::string httpclient_close(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    for (const ss::JsonField& fi : json.get_object()) {
        auto& name = fi.get_name();
        if ("httpclientHandle" == name) {
            handle = detail::get_json_handle(fi, "httpclientHandle");
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'httpclientHandle' not specified, data: [" + data + "]"));
    // get handle
    wilton_HttpClient* http = static_registry().remove(handle);
    if (nullptr == http) throw WiltonJsException(TRACEMSG(
            "Invalid 'httpclientHandle' parameter specified: [" + data + "]"));
    // call wilton
    char* err = wilton_HttpClient_close(http);
    if (nullptr != err) detail::throw_wilton_error(err, TRACEMSG(std::string(err) +
            "\nhttpclient_close error for input data: [" + data + "]"));
    return "{}";
}

std::string httpclient_execute(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    auto url = std::ref(EMPTY_STRING);
    auto request_data = std::ref(EMPTY_STRING);
    std::string metadata = EMPTY_STRING;
    for (const ss::JsonField& fi : json.get_object()) {
        auto& name = fi.get_name();
        if ("httpclientHandle" == name) {
            handle = detail::get_json_handle(fi, "httpclientHandle");
        } else if ("url" == name) {
            url = detail::get_json_string(fi, "url");
        } else if ("data" == name) {
            request_data = fi.get_string();
        } else if ("metadata" == name) {
            metadata = ss::dump_json_to_string(fi.get_value());
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'httpclientHandle' not specified, data: [" + data + "]"));
    if (url.get().empty()) throw WiltonJsException(TRACEMSG(
            "Required parameter 'url' not specified, data: [" + data + "]"));
    // get handle
    wilton_HttpClient* http = static_registry().remove(handle);
    if (nullptr == http) throw WiltonJsException(TRACEMSG(
            "Invalid 'httpclientHandle' parameter specified: [" + data + "]"));
    // call wilton
    char* out;
    int out_len;
    char* err = wilton_HttpClient_execute(http, url.get().c_str(), url.get().length(), 
            request_data.get().c_str(), request_data.get().length(), metadata.c_str(), metadata.length(),
            std::addressof(out), std::addressof(out_len));
    static_registry().put(http);
    if (nullptr != err) detail::throw_wilton_error(err, TRACEMSG(std::string(err) +
            "\nhttpclient_execute error for input data: [" + data + "]"));
    return detail::wrap_wilton_output(out, out_len);
}

std::string httpclient_send_temp_file(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    auto url = std::ref(EMPTY_STRING);
    auto filePath = std::ref(EMPTY_STRING);
    std::string metadata = EMPTY_STRING;
    for (const ss::JsonField& fi : json.get_object()) {
        auto& name = fi.get_name();
        if ("httpclientHandle" == name) {
            handle = detail::get_json_handle(fi, "httpclientHandle");
        } else if ("url" == name) {
            url = detail::get_json_string(fi, "url");
        } else if ("filePath" == name) {
            filePath = detail::get_json_string(fi, "filePath");
        } else if ("metadata" == name) {
            metadata = ss::dump_json_to_string(fi.get_value());
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'httpclientHandle' not specified, data: [" + data + "]"));
    if (url.get().empty()) throw WiltonJsException(TRACEMSG(
            "Required parameter 'url' not specified, data: [" + data + "]"));
    if (filePath.get().empty()) throw WiltonJsException(TRACEMSG(
            "Required parameter 'filePath' not specified, data: [" + data + "]"));    
    // get handle
    wilton_HttpClient* http = static_registry().remove(handle);
    if (nullptr == http) throw WiltonJsException(TRACEMSG(
            "Invalid 'httpclientHandle' parameter specified: [" + data + "]"));
    // call wilton
    char* out;
    int out_len;
    char* err = wilton_HttpClient_send_file(http, url.get().c_str(), url.get().length(), 
            filePath.get().c_str(), filePath.get().length(), metadata.c_str(), metadata.length(), 
            std::addressof(out), std::addressof(out_len),
            new std::string(filePath.get().data(), filePath.get().length()),
            [](void* ctx, int) {
                std::string* filePath_passed = static_cast<std::string*> (ctx);
                std::remove(filePath_passed->c_str());
                delete filePath_passed;
            });
    static_registry().put(http);
    if (nullptr != err) detail::throw_wilton_error(err, TRACEMSG(std::string(err) +
            "\nhttpclient_send_temp_file error for input data: [" + data + "]"));
    return detail::wrap_wilton_output(out, out_len);
}

}
