/* 
 * File:   wiltonjs_server.cpp
 * Author: alex
 *
 * Created on August 21, 2016, 1:14 PM
 */

#include "wiltonjs/wiltonjs.hpp"

#include <functional>
#include <list>
#include <memory>
#include <unordered_map>
#include <vector>

#include "staticlib/config.hpp"
#include "staticlib/serialization.hpp"

#include "wilton/wilton.h"

namespace wiltonjs {

namespace { // anonymous

namespace sc = staticlib::config;
namespace ss = staticlib::serialization;

const std::string EMPTY_STRING = "";

class HttpView {
public:    
    std::string method;
    std::string path;
    std::string module;

    HttpView(const HttpView&) = delete;

    HttpView& operator=(const HttpView&) = delete;

    HttpView(HttpView&& other) :
    method(std::move(other.method)),
    path(std::move(other.path)),
    module(std::move(other.module)) { }

    HttpView& operator=(HttpView&&) = delete;
    
    HttpView(const ss::JsonValue& json) {
        if (ss::JsonType::OBJECT != json.type()) throw WiltonJsException(TRACEMSG(
                "Invalid 'views' entry: must be an 'object'," +
                " entry: [" + ss::dump_json_to_string(json) + "]"));
        for (const ss::JsonField& fi : json.as_object()) {
            auto& name = fi.name();
            if ("method" == name) {
                method = detail::get_json_string(fi);
            } else if ("path" == name) {
                path = detail::get_json_string(fi);
            } else if ("module" == name) {
                module = detail::get_json_string(fi);
            } else {
                throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
            }
        }
    }
};

class HttpPathDeleter {
public:
    void operator()(wilton_HttpPath* path) {
        wilton_HttpPath_destroy(path);
    }
};

class GlobalRefDeleter {
public:
    void operator()(void* ref) {        
        detail::delete_wrapped_object(ref);
    }
};

class CtxEntry {
public:    
    void* gateway;
    void* modname;

    CtxEntry(const CtxEntry& other) :
    gateway(other.gateway),
    modname(other.modname) { }

    CtxEntry& operator=(const CtxEntry&) = delete;
    
    CtxEntry(void* gateway, void* modname) :
    gateway(gateway),
    modname(modname) { }
};


class ServerJniCtx {
    std::unique_ptr<void, GlobalRefDeleter> gateway;
    std::vector<std::unique_ptr<void, GlobalRefDeleter>> modules;
    // iterators must be permanent
    std::list<CtxEntry> entries;

public:
    ServerJniCtx(const ServerJniCtx&) = delete;
    
    ServerJniCtx& operator=(const ServerJniCtx&) = delete;

    ServerJniCtx(ServerJniCtx&& other) :
    gateway(std::move(other.gateway)),
    modules(std::move(other.modules)),
    entries(std::move(other.entries)) { }

    ServerJniCtx& operator=(ServerJniCtx&&) = delete;
    
    ServerJniCtx() { }
    
    ServerJniCtx(void* gateway, std::vector<HttpView>& views) :
    gateway(detail::wrap_object_permanent(gateway), GlobalRefDeleter()) {
        for (auto& vi : views) {
            void* str = detail::create_platform_string(vi.module);
            modules.emplace_back(detail::wrap_object_permanent(str), GlobalRefDeleter());
        }
    }
    
    void* get_gateway() {
        return gateway.get();
    }
    
    void* get_modname(size_t idx) {
        return modules[idx].get();
    }
    
    void add_entry(void* gateway, void* modname) {
        entries.emplace_back(CtxEntry(gateway, modname));
    }
    
    CtxEntry* get_last_entry() {
        return std::addressof(entries.back());
    }
    
};

class server_handle_registry {
    // reference to permanent non-destructable heap object
    std::unordered_map<wilton_Server*, ServerJniCtx>& registry;
    std::mutex mutex;

public:
    server_handle_registry() :
    registry(*(new std::unordered_map<wilton_Server*, ServerJniCtx>())) {}
    
    int64_t put(wilton_Server* ptr, ServerJniCtx&& ctx) {
        std::lock_guard<std::mutex> lock(mutex);
        auto pair = registry.emplace(ptr, std::move(ctx));
        return pair.second ? reinterpret_cast<int64_t> (ptr) : 0;
    }

    std::pair<wilton_Server*, ServerJniCtx> remove(int64_t handle) {
        std::lock_guard<std::mutex> lock(mutex);
        wilton_Server* ptr = reinterpret_cast<wilton_Server*> (handle);
        auto it = registry.find(ptr);
        if (registry.end() != it) {
            auto ctx = std::move(it->second);
            registry.erase(ptr);
            return std::make_pair(ptr, std::move(ctx));
        } else {
            return std::make_pair(nullptr, ServerJniCtx());
        }
    }
};

server_handle_registry& static_server_registry() {
    static server_handle_registry registry;
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

std::vector<HttpView> extract_views(ss::JsonValue& conf) {
    auto pair = conf.as_object_mutable();
    if (!pair.second) throw WiltonJsException(TRACEMSG(
            "Invalid configuration object specified: invalid type," +
            " conf: [" + ss::dump_json_to_string(conf) + "]"));
    std::vector<ss::JsonField>& fields = *pair.first;
    std::vector<HttpView> views;
    uint32_t i = 0;
    for (auto it = fields.begin(); it != fields.end(); ++it) {
        ss::JsonField& fi = *it;
        if ("views" == fi.name()) {
            if (ss::JsonType::ARRAY != fi.type()) throw WiltonJsException(TRACEMSG(
                    "Invalid configuration object specified: 'views' attr is not a list," +
                    " conf: [" + ss::dump_json_to_string(conf) + "]"));
            if (0 == fi.as_array().size()) throw WiltonJsException(TRACEMSG(
                    "Invalid configuration object specified: 'views' attr is am empty list," +
                    " conf: [" + ss::dump_json_to_string(conf) + "]"));
            for (auto& va : fi.as_array()) {
                if (ss::JsonType::OBJECT != va.type()) throw WiltonJsException(TRACEMSG(
                        "Invalid configuration object specified: 'views' is not a 'object'," +
                        "index: [" + sc::to_string(i) + "], conf: [" + ss::dump_json_to_string(conf) + "]"));
                views.emplace_back(HttpView(va));
            }
            // drop views attr and return immediately (iters are invalidated)
            fields.erase(it);
            return views;
        }
        i++;
    }
    throw WiltonJsException(TRACEMSG(
            "Invalid configuration object specified: 'views' list not specified," +
            " conf: [" + ss::dump_json_to_string(conf) + "]"));
}

std::vector<std::unique_ptr<wilton_HttpPath, HttpPathDeleter>> create_paths(
        const std::vector<HttpView>& views, ServerJniCtx& ctx) {
    // assert(views.size() == ctx.get_modules_names().size())
    std::vector<std::unique_ptr<wilton_HttpPath, HttpPathDeleter>> res;
    size_t idx = 0;
    for (auto& vi : views) {
        void* modname = ctx.get_modname(idx);
        ctx.add_entry(ctx.get_gateway(), modname);
        CtxEntry* ctx_to_pass = ctx.get_last_entry();
        wilton_HttpPath* ptr = nullptr;
        auto err = wilton_HttpPath_create(std::addressof(ptr), vi.method.c_str(), vi.method.length(),
                vi.path.c_str(), vi.path.length(), static_cast<void*>(ctx_to_pass), 
                [](void* vctx, wilton_Request* request) {
                    auto ctx_passed = static_cast<CtxEntry*>(vctx);
                    int64_t requestHandle = static_request_registry().put(request);
                    detail::invoke_gateway(ctx_passed->gateway, ctx_passed->modname, requestHandle);
                    static_request_registry().remove(requestHandle);
                });
        if (nullptr != err) throw WiltonJsException(TRACEMSG(err));
        res.emplace_back(ptr, HttpPathDeleter());
        idx++;       
    }
    return res;
}

std::vector<wilton_HttpPath*> wrap_paths(std::vector<std::unique_ptr<wilton_HttpPath, HttpPathDeleter>>& paths) {
    std::vector<wilton_HttpPath*> res;
    for (auto& pa : paths) {
        res.push_back(pa.get());
    }
    return res;
}

} // namespace

std::string server_create(const std::string& data, void* object) {
    if (nullptr == object) throw WiltonJsException(TRACEMSG(
            "Required parameter 'gateway' not specified"));
    ss::JsonValue json = ss::load_json_from_string(data);
    auto conf_in = ss::load_json_from_string(data);
    auto views = extract_views(conf_in);
    auto conf = ss::dump_json_to_string(conf_in);
    ServerJniCtx ctx{object, views};
    auto paths = create_paths(views, ctx);
    auto paths_pass = wrap_paths(paths);
    wilton_Server* server = nullptr;
    char* err = wilton_Server_create(std::addressof(server),
            conf.c_str(), conf.length(), paths_pass.data(), paths_pass.size());
    if (nullptr != err) detail::throw_wilton_error(err, TRACEMSG(std::string(err)));
    int64_t handle = static_server_registry().put(server, std::move(ctx));
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
    auto pa = static_server_registry().remove(handle);
    if (nullptr == pa.first) throw WiltonJsException(TRACEMSG(
            "Invalid 'serverHandle' parameter specified"));
    // call wilton
    char* err = wilton_Server_stop(pa.first);
    if (nullptr != err) {
        static_server_registry().put(pa.first, std::move(pa.second));
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
