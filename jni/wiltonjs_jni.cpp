/* 
 * File:   wiltonjs_jni.cpp
 * Author: alex
 *
 * Created on May 1, 2016, 12:08 AM
 */


#include "jni.h"

#include <mutex>
#include <string>
#include <unordered_set>

#include "staticlib/config.hpp"

#include "wilton/wilton.h"

#include "wiltonjs/wiltoncall.hpp"
#include "wiltonjs/wiltonjs.hpp"

#define WILTON_JNI_CLASS net_wiltonwebtoolkit_WiltonJni
#define WILTON_JNI_GATEWAY_INTERFACE "net/wiltonwebtoolkit/WiltonGateway"
#define WILTON_JNI_EXCEPTION_CLASS "net/wiltonwebtoolkit/WiltonException"

#define WILTON_JNI_PASTER(x,y) Java_ ## x ## _ ## y
#define WILTON_JNI_EVALUATOR(x,y) WILTON_JNI_PASTER(x,y)
#define WILTON_JNI_FUNCTION(fun) WILTON_JNI_EVALUATOR(WILTON_JNI_CLASS, fun)

namespace { // anonymous

namespace sc = staticlib::config;
namespace wj = wiltonjs;

// globals
JavaVM* JAVA_VM;
jclass GATEWAY_INTERFACE_CLASS;
jmethodID GATEWAY_CALLBACK_METHOD;
jclass EXCEPTION_CLASS;
jclass FILE_CLASS;
jmethodID FILE_CONSTRUCTOR_METHOD;
jmethodID FILE_DELETE_METHOD;

template<typename T>
class handle_registry {
    std::unordered_set<T*> registry;
    std::mutex mutex;
    
public:
    jlong put(T* ptr) {
        std::lock_guard<std::mutex> lock(mutex);
        auto pair = registry.insert(ptr);
        return pair.second ? reinterpret_cast<jlong>(ptr) : 0;
    }
    
    T* remove(jlong handle) {
        std::lock_guard<std::mutex> lock(mutex);
        T* ptr = reinterpret_cast<T*> (handle);
        auto erased = registry.erase(ptr);
        return 1 == erased ? ptr : nullptr;
    }
};

// registries
handle_registry<wilton_Server> REGISTRY_SERVERS;
handle_registry<wilton_Request> REGISTRY_REQUESTS;
handle_registry<wilton_ResponseWriter> REGISTRY_RESPONSE_WRITERS;
handle_registry<wilton_DBConnection> REGISTRY_DBCONNS;
handle_registry<wilton_DBTransaction> REGISTRY_DBTRANS;
handle_registry<wilton_HttpClient> REGISTRY_HTTPCLIENTS;

void log_error(const std::string& message) {
    static std::string error_level{"ERROR"};
    static std::string logger_name{"net.wiltonwebtoolkit.WiltonJni"};
    wilton_logger_log(error_level.c_str(), error_level.length(), logger_name.c_str(), logger_name.length(),
            message.c_str(), message.length());
}

std::string jstring_to_str(JNIEnv* env, jstring jstr) {
    const char* cstr = env->GetStringUTFChars(jstr, 0);
    size_t cstr_len = static_cast<size_t> (env->GetStringUTFLength(jstr));
    std::string res{cstr, cstr_len};
    env->ReleaseStringUTFChars(jstr, cstr);
    return res;
}

void send_system_error(jlong requestHandle, std::string errmsg) {
    wilton_Request* request = REGISTRY_REQUESTS.remove(requestHandle);
    if (nullptr != request) {
        std::string conf{R"({"statusCode": 500, "statusMessage": "Server Error"})"};
        wilton_Request_set_response_metadata(request, conf.c_str(), conf.length());
        wilton_Request_send_response(request, errmsg.c_str(), errmsg.length());
        REGISTRY_REQUESTS.put(request);
    }
}

void call_gateway(jobject gateway, jlong requestHandle) {        
    JNIEnv* env;
    auto getenv_err = JAVA_VM->GetEnv(reinterpret_cast<void**>(std::addressof(env)), JNI_VERSION_1_6);
    switch (getenv_err) {
    case JNI_OK:
        break;
    case JNI_EDETACHED:
        if (JNI_OK == JAVA_VM->AttachCurrentThread(std::addressof(env), nullptr)) { 
            break; 
        }
        // fall-through to report error to client
    default:
        send_system_error(requestHandle, TRACEMSG("System error"));
        return;
    }
    env->CallVoidMethod(gateway, GATEWAY_CALLBACK_METHOD, requestHandle);
    if (env->ExceptionOccurred()) {
        send_system_error(requestHandle, TRACEMSG("Gateway error"));
        env->ExceptionClear();
    }
    // https://groups.google.com/forum/#!topic/android-ndk/2H8z5grNqjo
//    JAVA_VM->DetachCurrentThread();
}

void delete_file(jstring filePath) {
    JNIEnv* env;
    auto getenv_err = JAVA_VM->GetEnv(reinterpret_cast<void**> (std::addressof(env)), JNI_VERSION_1_6);
    switch (getenv_err) {
    case JNI_OK:
        break;
    case JNI_EDETACHED:
        if (JNI_OK == JAVA_VM->AttachCurrentThread(std::addressof(env), nullptr)) {
            break;
        }
        // fall-through to report error to client
    default:
        log_error(TRACEMSG("System error occured during setting up the environment for" +
                " 'File.delete()' finalizer call of 'send_file' operation"));
        return;
    }
    jobject file = env->NewObject(FILE_CLASS, FILE_CONSTRUCTOR_METHOD, filePath);
    if (!env->ExceptionOccurred()) {
        env->CallVoidMethod(file, FILE_DELETE_METHOD);
        if (env->ExceptionOccurred()) {
            log_error(TRACEMSG("Cannot delete specified temporary file: [" + jstring_to_str(env, filePath) + "]" +
                    " during 'File.delete()' finalizer call of 'send_file' operation"));
        }
    } else {
        log_error(TRACEMSG("System error occured during creation of 'File' object for " +
                " deleting specified temporary file: [" + jstring_to_str(env, filePath) + "]" +
                " during 'File.delete()' finalizer call of 'send_file' operation"));
    }
    env->ExceptionClear();
    env->DeleteGlobalRef(filePath);
}

void register_wiltoncalls() {    
    wj::put_wilton_function("render_mustache", wj::render_mustache);
}

} // namespace

extern "C" {

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void*) {
    // wiltoncalls
    // todo: error reporting
    register_wiltoncalls();
    // env
    JNIEnv* env;
    auto err = vm->GetEnv(reinterpret_cast<void**>(std::addressof(env)), JNI_VERSION_1_6);
    if (JNI_OK != err) { return -1; }
    JAVA_VM = vm;
    // gateway
    jclass gatewayCLass = env->FindClass(WILTON_JNI_GATEWAY_INTERFACE);
    if (nullptr == gatewayCLass) { return -1; }
    GATEWAY_INTERFACE_CLASS = static_cast<jclass> (env->NewGlobalRef(gatewayCLass));
    if (nullptr == GATEWAY_INTERFACE_CLASS) { return -1; }
    GATEWAY_CALLBACK_METHOD = env->GetMethodID(GATEWAY_INTERFACE_CLASS, "gatewayCallback", "(J)V");
    if (nullptr == GATEWAY_CALLBACK_METHOD) { return -1; }
    jclass exClass = env->FindClass(WILTON_JNI_EXCEPTION_CLASS);
    if (nullptr == exClass) { return -1; }
    EXCEPTION_CLASS = static_cast<jclass> (env->NewGlobalRef(exClass));    
    // file
    jclass fileClass = env->FindClass("java/io/File");
    if (nullptr == fileClass) { return -1; }
    FILE_CLASS = static_cast<jclass> (env->NewGlobalRef(fileClass));
    if (nullptr == FILE_CLASS) { return -1; }
    FILE_CONSTRUCTOR_METHOD = env->GetMethodID(FILE_CLASS, "<init>", "(Ljava/lang/String;)V");
    if (nullptr == FILE_CONSTRUCTOR_METHOD) { return -1; }
    FILE_DELETE_METHOD = env->GetMethodID(FILE_CLASS, "delete", "()Z");
    if (nullptr == FILE_DELETE_METHOD) { return -1; }    
    return JNI_VERSION_1_6;
}

JNIEXPORT jstring JNICALL WILTON_JNI_FUNCTION(wiltoncall)
(JNIEnv* env, jclass, jstring name, jstring data, jobject object) {
    if (nullptr == name) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Null 'name' parameter specified").c_str());
        return nullptr;
    }
    if (nullptr == data) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Null 'data' parameter specified").c_str());
        return nullptr;
    }
    std::string name_string = jstring_to_str(env, name);
    std::string data_string = jstring_to_str(env, data);
    try {
        std::string res = wj::invoke_wilton_function(name_string, data_string, static_cast<void*>(object));
        return env->NewStringUTF(res.c_str());
    } catch (const std::exception& e) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG(e.what()).c_str());
    } catch (...) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("System error").c_str());
    }
    return nullptr;
}


// Server

JNIEXPORT jlong JNICALL WILTON_JNI_FUNCTION(createServer)
(JNIEnv* env, jclass, jobject gateway, jstring conf) {
    if (nullptr == gateway) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Null 'gateway' parameter specified").c_str());
        return 0; 
    }
    if (nullptr == conf) { 
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Null 'conf' parameter specified").c_str());
        return 0; 
    }
    // todo: fixme - delete somehow on server stop
    gateway = env->NewGlobalRef(gateway);
    
    wilton_Server* server;
    const char* conf_cstr = env->GetStringUTFChars(conf, 0);
    int conf_len = static_cast<int> (env->GetStringUTFLength(conf));
    char* err = wilton_Server_create(std::addressof(server),
            gateway,
            [](void* gateway_ctx, wilton_Request* request) {
                jobject gateway = static_cast<jobject>(gateway_ctx);
                jlong requestHandle = REGISTRY_REQUESTS.put(request);
                call_gateway(gateway, requestHandle);
                REGISTRY_REQUESTS.remove(requestHandle);
            }, conf_cstr, conf_len);
    env->ReleaseStringUTFChars(conf, conf_cstr);
    if (nullptr == err) {
        return REGISTRY_SERVERS.put(server);
    } else {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG(err).c_str());
        wilton_free(err);
        return 0;
    }
}

JNIEXPORT void JNICALL WILTON_JNI_FUNCTION(stopServer)
(JNIEnv* env, jclass, jlong serverHandle) {
    wilton_Server* server = REGISTRY_SERVERS.remove(serverHandle);
    if (nullptr == server) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Invalid 'serverHandle' parameter specified:" +
                " [" + sc::to_string(serverHandle) + "]").c_str());
        return; 
    }
    char* err = wilton_Server_stop(server);
    if (nullptr != err) {
        REGISTRY_SERVERS.put(server);
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG(err).c_str());
        wilton_free(err);
    }
}

jstring JNICALL WILTON_JNI_FUNCTION(getRequestMetadata)
(JNIEnv* env, jclass, jlong requestHandle) {
    wilton_Request* request = REGISTRY_REQUESTS.remove(requestHandle);
    if (nullptr == request) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Invalid 'requestHandle' parameter specified:" +
                " [" + sc::to_string(requestHandle) + "]").c_str());
        return nullptr; 
    }
    char* metadata;
    int metadata_len;
    char* err = wilton_Request_get_request_metadata(request, 
            std::addressof(metadata), std::addressof(metadata_len));
    REGISTRY_REQUESTS.put(request);
    if (nullptr == err) {
        // consider it nul-terminated
        jstring res = env->NewStringUTF(metadata);
        wilton_free(metadata);
        return res;
    } else {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG(err).c_str());
        wilton_free(err);
        return nullptr;
    }
}

jstring JNICALL WILTON_JNI_FUNCTION(getRequestData)
(JNIEnv* env, jclass, jlong requestHandle) {
    wilton_Request* request = REGISTRY_REQUESTS.remove(requestHandle);
    if (nullptr == request) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Invalid 'requestHandle' parameter specified:" +
                " [" + sc::to_string(requestHandle) + "]").c_str());
        return nullptr;
    }
    char* data;
    int data_len;
    char* err = wilton_Request_get_request_data(request,
            std::addressof(data), std::addressof(data_len));
    REGISTRY_REQUESTS.put(request);
    if (nullptr == err) {
        // consider it nul-terminated
        jstring res = env->NewStringUTF(data);
        wilton_free(data);
        return res;
    } else {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG(err).c_str());
        wilton_free(err);
        return nullptr;
    }
}

jstring JNICALL WILTON_JNI_FUNCTION(getRequestDataFilename)
(JNIEnv* env, jclass, jlong requestHandle) {
    wilton_Request* request = REGISTRY_REQUESTS.remove(requestHandle);
    if (nullptr == request) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Invalid 'requestHandle' parameter specified:" +
                " [" + sc::to_string(requestHandle) + "]").c_str());
        return nullptr;
    }
    char* filename;
    int filename_len;
    char* err = wilton_Request_get_request_data_filename(request,
            std::addressof(filename), std::addressof(filename_len));
    REGISTRY_REQUESTS.put(request);
    if (nullptr == err) {
        // consider it nul-terminated
        jstring res = env->NewStringUTF(filename);
        wilton_free(filename);
        return res;
    } else {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG(err).c_str());
        wilton_free(err);
        return nullptr;
    }
}

void JNICALL WILTON_JNI_FUNCTION(setResponseMetadata)
(JNIEnv* env, jclass, jlong requestHandle, jstring conf) {
    if (nullptr == conf) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Null 'conf' parameter specified").c_str());
        return; 
    }
    wilton_Request* request = REGISTRY_REQUESTS.remove(requestHandle);
    if (nullptr == request) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Invalid 'requestHandle' parameter specified:" +
                " [" + sc::to_string(requestHandle) + "]").c_str());
        return;
    }
    const char* conf_cstr = env->GetStringUTFChars(conf, 0);
    int conf_len = static_cast<int>(env->GetStringUTFLength(conf));
    char* err = wilton_Request_set_response_metadata(request, conf_cstr, conf_len);
    env->ReleaseStringUTFChars(conf, conf_cstr);
    REGISTRY_REQUESTS.put(request);
    if (nullptr != err) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG(err).c_str());
        wilton_free(err);
    }
}

JNIEXPORT void JNICALL WILTON_JNI_FUNCTION(sendResponse)
(JNIEnv* env, jclass, jlong requestHandle, jstring data) {
    if (nullptr == data) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Null 'data' parameter specified").c_str());
        return; 
    }
    wilton_Request* request = REGISTRY_REQUESTS.remove(requestHandle);
    if (nullptr == request) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Invalid 'requestHandle' parameter specified:" +
                " [" + sc::to_string(requestHandle) + "]").c_str());
        return;
    }
    const char* data_cstr = env->GetStringUTFChars(data, 0);
    int data_len = static_cast<int> (env->GetStringUTFLength(data));
    char* err = wilton_Request_send_response(request, data_cstr, data_len);
    env->ReleaseStringUTFChars(data, data_cstr);
    REGISTRY_REQUESTS.put(request);
    if (nullptr != err) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG(err).c_str());
        wilton_free(err);
    }
}

JNIEXPORT void JNICALL WILTON_JNI_FUNCTION(sendTempFile)
(JNIEnv* env, jclass, jlong requestHandle, jstring filePath) {
    if (nullptr == filePath) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Null 'filePath' parameter specified").c_str());
        return; 
    } 
    wilton_Request* request = REGISTRY_REQUESTS.remove(requestHandle);
    if (nullptr == request) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Invalid 'requestHandle' parameter specified:" +
                " [" + sc::to_string(requestHandle) + "]").c_str());        
        return;
    }
    const char* file_path_cstr = env->GetStringUTFChars(filePath, 0);
    int file_path_len = static_cast<int> (env->GetStringUTFLength(filePath));    
    char* err = wilton_Request_send_file(request, file_path_cstr, file_path_len, 
            env->NewGlobalRef(filePath), 
            [](void* ctx, int) {
                jstring filePath_passed = static_cast<jstring>(ctx);
                delete_file(filePath_passed);                
            });
    REGISTRY_REQUESTS.put(request);
    env->ReleaseStringUTFChars(filePath, file_path_cstr);
    if (nullptr != err) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG(err).c_str());
        wilton_free(err);
    }
}

JNIEXPORT void JNICALL WILTON_JNI_FUNCTION(sendMustache)
(JNIEnv* env, jclass, jlong requestHandle, jstring mustache_file_path, jstring values_json) {
    if (nullptr == mustache_file_path) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Null 'mustache_file_path' parameter specified").c_str());
        return; 
    }
    if (nullptr == values_json) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Null 'values_json' parameter specified").c_str());
        return;
    }
    wilton_Request* request = REGISTRY_REQUESTS.remove(requestHandle);
    if (nullptr == request) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Invalid 'requestHandle' parameter specified:" +
                " [" + sc::to_string(requestHandle) + "]").c_str());
        return;
    }
    const char* mustache_file_path_cstr = env->GetStringUTFChars(mustache_file_path, 0);
    int mustache_file_path_len = static_cast<int> (env->GetStringUTFLength(mustache_file_path));
    const char* values_json_cstr = env->GetStringUTFChars(values_json, 0);
    int values_json_len = static_cast<int> (env->GetStringUTFLength(values_json));    
    char* err = wilton_Request_send_mustache(request, mustache_file_path_cstr, mustache_file_path_len, 
            values_json_cstr, values_json_len);
    REGISTRY_REQUESTS.put(request);
    env->ReleaseStringUTFChars(mustache_file_path, mustache_file_path_cstr);
    env->ReleaseStringUTFChars(values_json, values_json_cstr);
    if (nullptr != err) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG(err).c_str());
        wilton_free(err);
    }
}

JNIEXPORT jlong JNICALL WILTON_JNI_FUNCTION(sendLater)
(JNIEnv* env, jclass, jlong requestHandle) {
    wilton_Request* request = REGISTRY_REQUESTS.remove(requestHandle);
    if (nullptr == request) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Invalid 'requestHandle' parameter specified:" +
                " [" + sc::to_string(requestHandle) + "]").c_str());
        return 0;
    }
    wilton_ResponseWriter* writer;
    char* err = wilton_Request_send_later(request, std::addressof(writer));
    REGISTRY_REQUESTS.put(request);
    if (nullptr == err) {
        return REGISTRY_RESPONSE_WRITERS.put(writer);
    } else {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG(err).c_str());
        wilton_free(err);
        return 0;
    }
}

JNIEXPORT void JNICALL WILTON_JNI_FUNCTION(sendWithResponseWriter)
(JNIEnv* env, jclass, jlong responseWriterHandle, jstring data) {
    if (nullptr == data) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Null 'data' parameter specified").c_str());
        return;
    }
    // note: won't be put back - one-off operation
    wilton_ResponseWriter* writer = REGISTRY_RESPONSE_WRITERS.remove(responseWriterHandle);
    if (nullptr == writer) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Invalid 'responseWriterHandle' parameter specified:" +
                " [" + sc::to_string(responseWriterHandle) + "]").c_str());
        return;
    }
    const char* data_cstr = env->GetStringUTFChars(data, 0);
    int data_len = static_cast<int> (env->GetStringUTFLength(data));
    char* err = wilton_ResponseWriter_send(writer, data_cstr, data_len);
    env->ReleaseStringUTFChars(data, data_cstr);
    if (nullptr != err) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG(err).c_str());
        wilton_free(err);
    }
}


// logging

JNIEXPORT void JNICALL WILTON_JNI_FUNCTION(appendLog)
(JNIEnv* env, jclass, jstring level, jstring logger, jstring message) {
    if (nullptr == level) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Null 'level' parameter specified").c_str());
        return; 
    }
    if (nullptr == logger) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Null 'logger' parameter specified").c_str());
        return;
    }
    if (nullptr == message) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Null 'message' parameter specified").c_str());
        return;
    }
    const char* level_cstr = env->GetStringUTFChars(level, 0);
    int level_len = static_cast<int> (env->GetStringUTFLength(level));
    const char* logger_cstr = env->GetStringUTFChars(logger, 0);
    int logger_len = static_cast<int> (env->GetStringUTFLength(logger));
    const char* message_cstr = env->GetStringUTFChars(message, 0);
    int message_len = static_cast<int> (env->GetStringUTFLength(message));
    char* err = wilton_logger_log(level_cstr, level_len, logger_cstr, logger_len, message_cstr, message_len);
    env->ReleaseStringUTFChars(level, level_cstr);
    env->ReleaseStringUTFChars(logger, logger_cstr);
    env->ReleaseStringUTFChars(message, message_cstr);
    if (nullptr != err) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG(err).c_str());
        wilton_free(err);
    }
}


// DB

JNIEXPORT jlong JNICALL WILTON_JNI_FUNCTION(openDbConnection)
(JNIEnv* env, jclass, jstring url) {
    if (nullptr == url) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Null 'url' parameter specified").c_str());
        return 0;
    }
    wilton_DBConnection* conn;
    const char* url_cstr = env->GetStringUTFChars(url, 0);
    int url_len = static_cast<int> (env->GetStringUTFLength(url));
    char* err = wilton_DBConnection_open(std::addressof(conn), url_cstr, url_len);
    env->ReleaseStringUTFChars(url, url_cstr);
    if (nullptr == err) {
        return REGISTRY_DBCONNS.put(conn);
    } else {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG(err).c_str());
        wilton_free(err);
        return 0;
    }
}

JNIEXPORT jstring JNICALL WILTON_JNI_FUNCTION(dbQuery)
(JNIEnv* env, jclass, jlong connectionHandle, jstring sql, jstring paramsJson) {
    if (nullptr == sql) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Null 'sql' parameter specified").c_str());
        return nullptr;
    }
    if (nullptr == paramsJson) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Null 'paramsJson' parameter specified").c_str());
        return nullptr;
    }
    wilton_DBConnection* conn = REGISTRY_DBCONNS.remove(connectionHandle);
    if (nullptr == conn) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Invalid 'connectionHandle' parameter specified:" +
                " [" + sc::to_string(connectionHandle) + "]").c_str());
        return nullptr;
    }
    const char* sql_cstr = env->GetStringUTFChars(sql, 0);
    int sql_len = static_cast<int> (env->GetStringUTFLength(sql));
    const char* paramsJson_cstr = env->GetStringUTFChars(paramsJson, 0);
    int paramsJson_len = static_cast<int> (env->GetStringUTFLength(paramsJson));
    char* data;
    int data_len;
    char* err = wilton_DBConnection_query(conn, sql_cstr, sql_len, paramsJson_cstr, paramsJson_len, 
            std::addressof(data), std::addressof(data_len));
    REGISTRY_DBCONNS.put(conn);
    env->ReleaseStringUTFChars(sql, sql_cstr);
    env->ReleaseStringUTFChars(paramsJson, paramsJson_cstr);
    if (nullptr == err) {
        // consider it nul-terminated
        jstring res = env->NewStringUTF(data);
        wilton_free(data);
        return res;
    } else {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG(err).c_str());
        wilton_free(err);
        return nullptr;
    }
}

JNIEXPORT void JNICALL WILTON_JNI_FUNCTION(dbExecute)
(JNIEnv* env, jclass, jlong connectionHandle, jstring sql, jstring paramsJson) {
    if (nullptr == sql) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Null 'sql' parameter specified").c_str());
        return;
    }
    if (nullptr == paramsJson) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Null 'paramsJson' parameter specified").c_str());
        return;
    }
    wilton_DBConnection* conn = REGISTRY_DBCONNS.remove(connectionHandle);
    if (nullptr == conn) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Invalid 'connectionHandle' parameter specified:" +
                " [" + sc::to_string(connectionHandle) + "]").c_str());
        return;
    }
    const char* sql_cstr = env->GetStringUTFChars(sql, 0);
    int sql_len = static_cast<int> (env->GetStringUTFLength(sql));
    const char* paramsJson_cstr = env->GetStringUTFChars(paramsJson, 0);
    int paramsJson_len = static_cast<int> (env->GetStringUTFLength(paramsJson));
    char* err = wilton_DBConnection_execute(conn, sql_cstr, sql_len, paramsJson_cstr, paramsJson_len);
    REGISTRY_DBCONNS.put(conn);
    env->ReleaseStringUTFChars(sql, sql_cstr);
    env->ReleaseStringUTFChars(paramsJson, paramsJson_cstr);
    if (nullptr != err) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG(err).c_str());
        wilton_free(err);
    }
}

JNIEXPORT void JNICALL WILTON_JNI_FUNCTION(closeDbConnection)
(JNIEnv* env, jclass, jlong connectionHandle) {
    wilton_DBConnection* conn = REGISTRY_DBCONNS.remove(connectionHandle);
    if (nullptr == conn) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Invalid 'connectionHandle' parameter specified:" +
                " [" + sc::to_string(connectionHandle) + "]").c_str());
        return;
    }
    char* err = wilton_DBConnection_close(conn);
    if (nullptr != err) {
        REGISTRY_DBCONNS.put(conn);
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG(err).c_str());
        wilton_free(err);
    }    
}

JNIEXPORT jlong JNICALL WILTON_JNI_FUNCTION(startDbTransaction)
(JNIEnv* env, jclass, jlong connectionHandle) {
    wilton_DBConnection* conn = REGISTRY_DBCONNS.remove(connectionHandle);
    if (nullptr == conn) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Invalid 'connectionHandle' parameter specified:" +
                " [" + sc::to_string(connectionHandle) + "]").c_str());
        return 0;
    }
    wilton_DBTransaction* tran;
    char* err = wilton_DBTransaction_start(conn, std::addressof(tran));
    REGISTRY_DBCONNS.put(conn);
    if (nullptr == err) {
        return REGISTRY_DBTRANS.put(tran);
    } else {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG(err).c_str());
        wilton_free(err);
        return 0;
    }
}

JNIEXPORT void JNICALL WILTON_JNI_FUNCTION(commitDbTransaction)
(JNIEnv* env, jclass, jlong transactionHandle) {
    wilton_DBTransaction* tran = REGISTRY_DBTRANS.remove(transactionHandle);
    if (nullptr == tran) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Invalid 'transactionHandle' parameter specified:" +
                " [" + sc::to_string(transactionHandle) + "]").c_str());
        return;
    }
    char* err = wilton_DBTransaction_commit(tran);
    if (nullptr != err) {
        REGISTRY_DBTRANS.put(tran);
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG(err).c_str());
        wilton_free(err);
    }
}

JNIEXPORT void JNICALL WILTON_JNI_FUNCTION(rollbackDbTransaction)
(JNIEnv* env, jclass, jlong transactionHandle) {
    wilton_DBTransaction* tran = REGISTRY_DBTRANS.remove(transactionHandle);
    if (nullptr == tran) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Invalid 'transactionHandle' parameter specified:" +
                " [" + sc::to_string(transactionHandle) + "]").c_str());
        return;
    }
    char* err = wilton_DBTransaction_rollback(tran);
    if (nullptr != err) {
        REGISTRY_DBTRANS.put(tran);
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG(err).c_str());
        wilton_free(err);
    }
}


// HttpClient

JNIEXPORT jlong JNICALL WILTON_JNI_FUNCTION(createHttpClient)
(JNIEnv* env, jclass, jstring conf) {
    if (nullptr == conf) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Null 'conf' parameter specified").c_str());
        return 0;
    }
    wilton_HttpClient* http;
    const char* conf_cstr = env->GetStringUTFChars(conf, 0);
    int conf_len = static_cast<int> (env->GetStringUTFLength(conf));
    char* err = wilton_HttpClient_create(std::addressof(http), conf_cstr, conf_len);
    env->ReleaseStringUTFChars(conf, conf_cstr);
    if (nullptr == err) {
        return REGISTRY_HTTPCLIENTS.put(http);
    } else {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG(err).c_str());
        wilton_free(err);
        return 0;
    }
}

JNIEXPORT void JNICALL WILTON_JNI_FUNCTION(closeHttpClient)
(JNIEnv* env, jclass, jlong httpClientHandle) {
    wilton_HttpClient* http = REGISTRY_HTTPCLIENTS.remove(httpClientHandle);
    if (nullptr == http) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Invalid 'httpClientHandle' parameter specified:" +
                " [" + sc::to_string(httpClientHandle) + "]").c_str());
        return;
    }
    char* err = wilton_HttpClient_close(http);
    if (nullptr != err) {
        REGISTRY_HTTPCLIENTS.put(http);
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG(err).c_str());
        wilton_free(err);
    }
}

JNIEXPORT jstring JNICALL WILTON_JNI_FUNCTION(httpExecute)
(JNIEnv* env, jclass, jlong httpClientHandle, jstring url, jstring data, jstring metadata) {
    if (nullptr == url) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Null 'url' parameter specified").c_str());
        return nullptr;
    }
    if (nullptr == data) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Null 'data' parameter specified").c_str());
        return nullptr;
    }
    if (nullptr == metadata) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Null 'metadata' parameter specified").c_str());
        return nullptr;
    }
    wilton_HttpClient* http = REGISTRY_HTTPCLIENTS.remove(httpClientHandle);
    if (nullptr == http) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Invalid 'httpClientHandle' parameter specified:" +
                " [" + sc::to_string(httpClientHandle) + "]").c_str());
        return nullptr;
    }
    const char* url_cstr = env->GetStringUTFChars(url, 0);
    int url_len = static_cast<int> (env->GetStringUTFLength(url));
    const char* data_cstr = env->GetStringUTFChars(data, 0);
    int data_len = static_cast<int> (env->GetStringUTFLength(data));
    const char* metadata_cstr = env->GetStringUTFChars(metadata, 0);
    int metadata_len = static_cast<int> (env->GetStringUTFLength(metadata));
    char* data_out;
    int data_out_len;
    char* err = wilton_HttpClient_execute(http, url_cstr, url_len, data_cstr, data_len, metadata_cstr, metadata_len,
            std::addressof(data_out), std::addressof(data_out_len));
    REGISTRY_HTTPCLIENTS.put(http);
    env->ReleaseStringUTFChars(url, url_cstr);
    env->ReleaseStringUTFChars(data, data_cstr);
    env->ReleaseStringUTFChars(metadata, metadata_cstr);
    if (nullptr == err) {
        // consider it nul-terminated
        jstring res = env->NewStringUTF(data_out);
        wilton_free(data_out);
        return res;
    } else {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG(err).c_str());
        wilton_free(err);
        return nullptr;
    }
}

JNIEXPORT jstring JNICALL WILTON_JNI_FUNCTION(httpSendTempFile)
(JNIEnv* env, jclass, jlong httpClientHandle, jstring url, jstring filePath, jstring metadata) {
    if (nullptr == url) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Null 'url' parameter specified").c_str());
        return nullptr;
    }
    if (nullptr == filePath) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Null 'filePath' parameter specified").c_str());
        return nullptr;
    }
    if (nullptr == metadata) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Null 'metadata' parameter specified").c_str());
        return nullptr;
    }
    wilton_HttpClient* http = REGISTRY_HTTPCLIENTS.remove(httpClientHandle);
    if (nullptr == http) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG("Invalid 'httpClientHandle' parameter specified:" +
                " [" + sc::to_string(httpClientHandle) + "]").c_str());
        return nullptr;
    }
    const char* url_cstr = env->GetStringUTFChars(url, 0);
    int url_len = static_cast<int> (env->GetStringUTFLength(url));
    const char* filePath_cstr = env->GetStringUTFChars(filePath, 0);
    int filePath_len = static_cast<int> (env->GetStringUTFLength(filePath));
    const char* metadata_cstr = env->GetStringUTFChars(metadata, 0);
    int metadata_len = static_cast<int> (env->GetStringUTFLength(metadata));
    char* data_out;
    int data_out_len;
    char* err = wilton_HttpClient_send_file(http, url_cstr, url_len, filePath_cstr, filePath_len,
            metadata_cstr, metadata_len, std::addressof(data_out), std::addressof(data_out_len),
            env->NewGlobalRef(filePath), 
            [](void* ctx, int) {
                jstring filePath_passed = static_cast<jstring> (ctx);
                delete_file(filePath_passed);
            });
    REGISTRY_HTTPCLIENTS.put(http);
    env->ReleaseStringUTFChars(url, url_cstr);
    env->ReleaseStringUTFChars(filePath, filePath_cstr);
    env->ReleaseStringUTFChars(metadata, metadata_cstr);
    if (nullptr == err) {
        // consider it nul-terminated
        jstring res = env->NewStringUTF(data_out);
        wilton_free(data_out);
        return res;
    } else {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG(err).c_str());
        wilton_free(err);
        return nullptr;
    }
}


} // C
