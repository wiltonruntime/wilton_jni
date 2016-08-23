/* 
 * File:   wiltonjs_httpclient.cpp
 * Author: alex
 *
 * Created on August 21, 2016, 1:14 PM
 */

#include "wiltonjs/wiltonjs.hpp"

#include "staticlib/serialization.hpp"

#include "wilton/wilton.h"

namespace wiltonjs {

namespace { // anonymous

namespace ss = staticlib::serialization;

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
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = json.get("httpclientHandle").get_integer();
    wilton_HttpClient* http = static_registry().remove(handle);
    if (nullptr == http) throw WiltonJsException(TRACEMSG(
            "Invalid 'httpclientHandle' parameter specified: [" + data + "]"));
    char* err = wilton_HttpClient_close(http);
    if (nullptr != err) detail::throw_wilton_error(err, TRACEMSG(std::string(err) +
            "\nhttpclient_close error for input data: [" + data + "]"));
    return "{}";
}

std::string httpclient_execute(const std::string& data, void*) {
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = json.get("httpclientHandle").get_integer();
    const std::string& url = json.get("url").get_string();
    const std::string& request_data = json.get("data").get_string();
    const std::string& metadata = ss::dump_json_to_string(json.get("metadata"));
    wilton_HttpClient* http = static_registry().remove(handle);
    if (nullptr == http) throw WiltonJsException(TRACEMSG(
            "Invalid 'httpclientHandle' parameter specified: [" + data + "]"));
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
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = json.get("httpclientHandle").get_integer();
    const std::string& url = json.get("url").get_string();
    const std::string& filePath = json.get("filePath").get_string();
    const std::string& metadata = ss::dump_json_to_string(json.get("metadata"));
    wilton_HttpClient* http = static_registry().remove(handle);
    if (nullptr == http) throw WiltonJsException(TRACEMSG(
            "Invalid 'httpclientHandle' parameter specified: [" + data + "]"));
    char* out;
    int out_len;
    char* err = wilton_HttpClient_send_file(http, url.c_str(), url.length(), 
            filePath.c_str(), filePath.length(), metadata.c_str(), metadata.length(), 
            std::addressof(out), std::addressof(out_len),
            new std::string(filePath),
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
