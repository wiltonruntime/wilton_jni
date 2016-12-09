/* 
 * File:   wiltonjs_server.cpp
 * Author: alex
 *
 * Created on August 21, 2016, 1:14 PM
 */

#include "wiltonjs/wiltonjs.hpp"

#include <functional>

#include "jni.h"

#include "staticlib/serialization.hpp"

#include "wilton/wilton.h"

namespace wiltonjs {

namespace { // anonymous

namespace ss = staticlib::serialization;

const std::string EMPTY_STRING = "";

detail::handle_registry<wilton_Server>& static_server_registry() {
    static detail::handle_registry<wilton_Server> registry;
    return registry;
}

detail::handle_registry<wilton_Request>& static_request_registry() {
    static detail::handle_registry<wilton_Request> registry;
    return registry;
}

detail::handle_registry<wilton_ResponseWriter>& static_response_writer_registry() {
    static detail::handle_registry<wilton_ResponseWriter> registry;
    return registry;
}

void send_system_error(int64_t requestHandle, std::string errmsg) {
    wilton_Request* request = static_request_registry().remove(requestHandle);
    if (nullptr != request) {
        std::string conf{R"({"statusCode": 500, "statusMessage": "Server Error"})"};
        wilton_Request_set_response_metadata(request, conf.c_str(), conf.length());
        wilton_Request_send_response(request, errmsg.c_str(), errmsg.length());
        static_request_registry().put(request);
    }
}

void call_gateway(jobject gateway, int64_t requestHandle) {
    JNIEnv* env = nullptr;
    try {
        env = static_cast<JNIEnv*> (detail::get_jni_env());
        env->CallVoidMethod(gateway, static_cast<jmethodID> (detail::get_gateway_method()), requestHandle);
        if (env->ExceptionOccurred()) {
            std::string msg = TRACEMSG("Gateway error");
            detail::log_error(msg);
            send_system_error(requestHandle, msg);
            env->ExceptionClear();
        }
    } catch (const std::exception& e) {
        std::string msg = TRACEMSG(e.what() + "\nGateway error");
        detail::log_error(msg);
        send_system_error(requestHandle, msg);
    }
}

} // namespace

std::string server_create(const std::string& data, void* object) {
    if (nullptr == object) throw WiltonJsException(TRACEMSG(
            "Required parameter 'gateway' not specified"));
    JNIEnv* env = static_cast<JNIEnv*> (detail::get_jni_env());
    jobject gateway_local = static_cast<jobject> (object);
    // todo: fixme - delete somehow on server stop
    jobject gateway = env->NewGlobalRef(gateway_local);
    wilton_Server* server;
    char* err = wilton_Server_create(std::addressof(server),
            gateway,
            [](void* gateway_ctx, wilton_Request* request) {
                jobject gateway = static_cast<jobject> (gateway_ctx);
                int64_t requestHandle = static_request_registry().put(request);
                call_gateway(gateway, requestHandle);
                static_request_registry().remove(requestHandle);
            }, data.c_str(), data.length());
    if (nullptr != err) detail::throw_wilton_error(err, TRACEMSG(std::string(err)));
    int64_t handle = static_server_registry().put(server);
    return ss::dump_json_to_string({
        { "serverHandle", handle}
    });
}

std::string server_stop(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("serverHandle" == name) {
            handle = detail::get_json_int(fi);
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'serverHandle' not specified"));
    // get handle
    wilton_Server* server = static_server_registry().remove(handle);
    if (nullptr == server) throw WiltonJsException(TRACEMSG(
            "Invalid 'serverHandle' parameter specified"));
    // call wilton
    char* err = wilton_Server_stop(server);
    if (nullptr != err) {
        static_server_registry().put(server);
        detail::throw_wilton_error(err, TRACEMSG(std::string(err)));
    }
    return "{}";
}

std::string request_get_metadata(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("requestHandle" == name) {
            handle = detail::get_json_int(fi);
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'requestHandle' not specified"));
    // get handle
    wilton_Request* request = static_request_registry().remove(handle);
    if (nullptr == request) throw WiltonJsException(TRACEMSG(
            "Invalid 'requestHandle' parameter specified"));
    // call wilton
    char* out;
    int out_len;
    char* err = wilton_Request_get_request_metadata(request,
            std::addressof(out), std::addressof(out_len));
    static_request_registry().put(request);
    if (nullptr != err) {
        detail::throw_wilton_error(err, TRACEMSG(std::string(err)));
    }
    return detail::wrap_wilton_output(out, out_len);
}

std::string request_get_data(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("requestHandle" == name) {
            handle = detail::get_json_int(fi);
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'requestHandle' not specified"));
    // get handle
    wilton_Request* request = static_request_registry().remove(handle);
    if (nullptr == request) throw WiltonJsException(TRACEMSG(
            "Invalid 'requestHandle' parameter specified"));
    // call wilton
    char* out;
    int out_len;
    char* err = wilton_Request_get_request_data(request,
            std::addressof(out), std::addressof(out_len));
    static_request_registry().put(request);
    if (nullptr != err) detail::throw_wilton_error(err, TRACEMSG(std::string(err)));
    return detail::wrap_wilton_output(out, out_len);
}

std::string request_get_data_filename(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("requestHandle" == name) {
            handle = detail::get_json_int(fi);
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'requestHandle' not specified"));
    // get handle
    wilton_Request* request = static_request_registry().remove(handle);
    if (nullptr == request) throw WiltonJsException(TRACEMSG(
            "Invalid 'requestHandle' parameter specified"));
    // call wilton
    char* out;
    int out_len;
    char* err = wilton_Request_get_request_data_filename(request,
            std::addressof(out), std::addressof(out_len));
    static_request_registry().put(request);
    if (nullptr != err) detail::throw_wilton_error(err, TRACEMSG(std::string(err)));
    return detail::wrap_wilton_output(out, out_len);
}

std::string request_set_response_metadata(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    std::string metadata = EMPTY_STRING;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("requestHandle" == name) {
            handle = detail::get_json_int(fi);
        } else if ("metadata" == name) {
            metadata = ss::dump_json_to_string(fi.value());
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'requestHandle' not specified"));
    if (metadata.empty()) throw WiltonJsException(TRACEMSG(
            "Required parameter 'metadata' not specified"));
    // get handle
    wilton_Request* request = static_request_registry().remove(handle);
    if (nullptr == request) throw WiltonJsException(TRACEMSG(
            "Invalid 'requestHandle' parameter specified"));
    // call wilton
    char* err = wilton_Request_set_response_metadata(request, metadata.c_str(), metadata.length());
    static_request_registry().put(request);
    if (nullptr != err) detail::throw_wilton_error(err, TRACEMSG(std::string(err)));
    return "{}";
}

std::string request_send_response(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    auto rdata = std::ref(EMPTY_STRING);
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("requestHandle" == name) {
            handle = detail::get_json_int(fi);
        } else if ("data" == name) {
            rdata = fi.as_string();
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'requestHandle' not specified"));
    const std::string& request_data = rdata.get().empty() ? "{}" : rdata.get();
    // get handle
    wilton_Request* request = static_request_registry().remove(handle);
    if (nullptr == request) throw WiltonJsException(TRACEMSG(
            "Invalid 'requestHandle' parameter specified"));
    // call wilton
    char* err = wilton_Request_send_response(request, request_data.c_str(), request_data.length());
    static_request_registry().put(request);
    if (nullptr != err) detail::throw_wilton_error(err, TRACEMSG(std::string(err)));
    return "{}";
}

std::string request_send_temp_file(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    std::string file = EMPTY_STRING;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("requestHandle" == name) {
            handle = detail::get_json_int(fi);
        } else if ("filePath" == name) {
            file = detail::get_json_string(fi);
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'requestHandle' not specified"));
    if (file.empty()) throw WiltonJsException(TRACEMSG(
            "Required parameter 'filePath' not specified"));
    // get handle
    wilton_Request* request = static_request_registry().remove(handle);
    if (nullptr == request) throw WiltonJsException(TRACEMSG(
            "Invalid 'requestHandle' parameter specified"));
    // call wilton
    char* err = wilton_Request_send_file(request, file.c_str(), file.length(),
            new std::string(file.data(), file.length()),
            [](void* ctx, int) {
                std::string* filePath_passed = static_cast<std::string*> (ctx);
                std::remove(filePath_passed->c_str());
                delete filePath_passed;
            });
    static_request_registry().put(request);
    if (nullptr != err) detail::throw_wilton_error(err, TRACEMSG(std::string(err)));
    return "{}";
}

std::string request_send_mustache(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    auto rfile = std::ref(EMPTY_STRING);
    std::string values = EMPTY_STRING;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("requestHandle" == name) {
            handle = detail::get_json_int(fi);
        } else if ("mustacheFilePath" == name) {
            rfile = detail::get_json_string(fi);
        } else if ("values" == name) {
            values = ss::dump_json_to_string(fi.value());
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'requestHandle' not specified"));
    if (rfile.get().empty()) throw WiltonJsException(TRACEMSG(
            "Required parameter 'mustacheFilePath' not specified"));
    if (values.empty()) {
        values = "{}";
    }
    const std::string& file = rfile.get();
    // get handle
    wilton_Request* request = static_request_registry().remove(handle);
    if (nullptr == request) throw WiltonJsException(TRACEMSG(
            "Invalid 'requestHandle' parameter specified"));
    // call wilton
    char* err = wilton_Request_send_mustache(request, file.c_str(), file.length(),
            values.c_str(), values.length());
    static_request_registry().put(request);
    if (nullptr != err) detail::throw_wilton_error(err, TRACEMSG(std::string(err)));
    return "{}";
}

std::string request_send_later(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("requestHandle" == name) {
            handle = detail::get_json_int(fi);
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'requestHandle' not specified"));
    // get handle
    wilton_Request* request = static_request_registry().remove(handle);
    if (nullptr == request) throw WiltonJsException(TRACEMSG(
            "Invalid 'requestHandle' parameter specified"));
    // call wilton
    wilton_ResponseWriter* writer;
    char* err = wilton_Request_send_later(request, std::addressof(writer));
    static_request_registry().put(request);
    if (nullptr != err) detail::throw_wilton_error(err, TRACEMSG(std::string(err)));
    int64_t rwhandle = static_response_writer_registry().put(writer);
    return ss::dump_json_to_string({
        { "responseWriterHandle", rwhandle}
    });
}

std::string request_send_with_response_writer(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    auto rdata = std::ref(EMPTY_STRING);
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("responseWriterHandle" == name) {
            handle = detail::get_json_int(fi);
        } else if ("data" == name) {
            rdata = fi.as_string();
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'responseWriterHandle' not specified"));
    const std::string& request_data = rdata.get().empty() ? "{}" : rdata.get();
    // get handle, note: won't be put back - one-off operation   
    wilton_ResponseWriter* writer = static_response_writer_registry().remove(handle);
    if (nullptr == writer) throw WiltonJsException(TRACEMSG(
            "Invalid 'responseWriterHandle' parameter specified"));
    // call wilton
    char* err = wilton_ResponseWriter_send(writer, request_data.c_str(), request_data.length());
    if (nullptr != err) detail::throw_wilton_error(err, TRACEMSG(std::string(err)));
    return "{}";
}



} // namespace
