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
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("httpclientHandle" == name) {
            handle = detail::get_json_handle(fi);
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
    if (nullptr != err) {
        static_registry().put(http);
        detail::throw_wilton_error(err, TRACEMSG(std::string(err) +
            "\nhttpclient_close error for input data: [" + data + "]"));
    }
    return "{}";
}

std::string httpclient_execute(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    auto rurl = std::ref(EMPTY_STRING);
    auto rdata = std::ref(EMPTY_STRING);
    std::string metadata = EMPTY_STRING;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("httpclientHandle" == name) {
            handle = detail::get_json_handle(fi);
        } else if ("url" == name) {
            rurl = detail::get_json_string(fi);
        } else if ("data" == name) {
            rdata = fi.as_string();
        } else if ("metadata" == name) {
            metadata = ss::dump_json_to_string(fi.value());
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'httpclientHandle' not specified, data: [" + data + "]"));
    if (rurl.get().empty()) throw WiltonJsException(TRACEMSG(
            "Required parameter 'url' not specified, data: [" + data + "]"));
    const std::string& url = rurl.get();
    const std::string& request_data = rdata.get();
    // get handle
    wilton_HttpClient* http = static_registry().remove(handle);
    if (nullptr == http) throw WiltonJsException(TRACEMSG(
            "Invalid 'httpclientHandle' parameter specified: [" + data + "]"));
    // call wilton
    char* out;
    int out_len;
    char* err = wilton_HttpClient_execute(http, url.c_str(), url.length(), 
            request_data.c_str(), request_data.length(), metadata.c_str(), metadata.length(),
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
    auto rurl = std::ref(EMPTY_STRING);
    auto rfile = std::ref(EMPTY_STRING);
    std::string metadata = EMPTY_STRING;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("httpclientHandle" == name) {
            handle = detail::get_json_handle(fi);
        } else if ("url" == name) {
            rurl = detail::get_json_string(fi);
        } else if ("filePath" == name) {
            rfile = detail::get_json_string(fi);
        } else if ("metadata" == name) {
            metadata = ss::dump_json_to_string(fi.value());
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'httpclientHandle' not specified, data: [" + data + "]"));
    if (rurl.get().empty()) throw WiltonJsException(TRACEMSG(
            "Required parameter 'url' not specified, data: [" + data + "]"));
    if (rfile.get().empty()) throw WiltonJsException(TRACEMSG(
            "Required parameter 'filePath' not specified, data: [" + data + "]"));
    const std::string& url = rurl.get();
    const std::string& file_path = rfile.get();
    // get handle
    wilton_HttpClient* http = static_registry().remove(handle);
    if (nullptr == http) throw WiltonJsException(TRACEMSG(
            "Invalid 'httpclientHandle' parameter specified: [" + data + "]"));
    // call wilton
    char* out;
    int out_len;
    char* err = wilton_HttpClient_send_file(http, url.c_str(), url.length(), 
            file_path.c_str(), file_path.length(), metadata.c_str(), metadata.length(), 
            std::addressof(out), std::addressof(out_len),
            new std::string(file_path.data(), file_path.length()),
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
