/* 
 * File:   wilton_jni.cpp
 * Author: alex
 *
 * Created on May 1, 2016, 12:08 AM
 */


#include "jni.h"

#include <string>

#include "staticlib/config.hpp"

#include "wilton/wilton.h"

// todo: compile time check
#define WILTON_JNI_CLASS net_wiltonwebtoolkit_WiltonJni
#define WILTON_JNI_GATEWAY_INTERFACE "net/wiltonwebtoolkit/WiltonGateway"
#define WILTON_JNI_EXCEPTION_CLASS "net/wiltonwebtoolkit/WiltonException"

#define WILTON_JNI_PASTER(x,y) Java_ ## x ## _ ## y
#define WILTON_JNI_EVALUATOR(x,y) WILTON_JNI_PASTER(x,y)
#define WILTON_JNI_FUNCTION(fun) WILTON_JNI_EVALUATOR(WILTON_JNI_CLASS, fun)

namespace { // anonymous

namespace sc = staticlib::config;

// globals
JavaVM* JAVA_VM;
jclass GATEWAY_INTERFACE_CLASS;
jmethodID GATEWAY_CALLBACK_METHOD;
jclass FILE_CLASS;
jmethodID FILE_CONSTRUCTOR_METHOD;
jmethodID FILE_DELETE_METHOD;

void throwException(JNIEnv* env, const char* message) {
    jclass exClass = env->FindClass(WILTON_JNI_EXCEPTION_CLASS);
    std::string msg = TRACEMSG(std::string() + message + "\nC API Error");
    env->ThrowNew(exClass, msg.c_str());
}

void log_error(const std::string& message) {
    static std::string error_level{"ERROR"};
    static std::string logger_name{"net.wiltonwebtoolkit.HttpServerJni"};
    wilton_log(error_level.c_str(), error_level.length(), logger_name.c_str(), logger_name.length(),
            message.c_str(), message.length());
}

// handles validation may be added here later

wilton_Server* serverFromHandle(JNIEnv*, jlong handle) {
    return reinterpret_cast<wilton_Server*> (handle);
}

wilton_Request* requestFromHandle(JNIEnv*, jlong requestHandle) {
    return reinterpret_cast<wilton_Request*> (requestHandle);
}

wilton_DBConnection* connectionFromHandle(JNIEnv*, jlong connectionHandle) {
    return reinterpret_cast<wilton_DBConnection*> (connectionHandle);
}

wilton_DBTransaction* transactionFromHandle(JNIEnv*, jlong transactionHandle) {
    return reinterpret_cast<wilton_DBTransaction*> (transactionHandle);
}

void callGateway(jobject gateway, jlong requestHandle) {
    // todo: fixme line
    wilton_Request* request = requestFromHandle(nullptr, requestHandle);
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
        std::string conf{R"({"statusCode": 500, "statusMessage": "Server Error"})"};
        wilton_Request_set_response_metadata(request, conf.c_str(), conf.length());
        std::string errmsg{TRACEMSG("System error")};
        wilton_Request_send_response(request, errmsg.c_str(), errmsg.length());
    }
    env->CallVoidMethod(gateway, GATEWAY_CALLBACK_METHOD, requestHandle);
    env->ExceptionClear();
    // https://groups.google.com/forum/#!topic/android-ndk/2H8z5grNqjo
//    JAVA_VM->DetachCurrentThread();
}

// todo: file name logging
void deleteFile(jstring filePath) {
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
        log_error("System error occured during setting up environment for 'send_file' 'File.delete()' finalizer call");
        return;
    }
    jobject file = env->NewObject(FILE_CLASS, FILE_CONSTRUCTOR_METHOD, filePath);
    if (!env->ExceptionOccurred()) {
        env->CallVoidMethod(file, FILE_DELETE_METHOD);
        if (env->ExceptionOccurred()) {
            log_error("System error during 'File.delete()' finalizer call for 'send_file'");
        }
    } else {
        log_error("System error during 'File.<init>()' finalizer call for 'send_file'");
    }
    env->ExceptionClear();
    env->DeleteGlobalRef(filePath);
}

} // namespace

extern "C" {

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void*) {    
    JNIEnv* env;
    auto err = vm->GetEnv(reinterpret_cast<void**>(std::addressof(env)), JNI_VERSION_1_6);
    if (JNI_OK != err) { return -1; }
    JAVA_VM = vm;
    // gateway
    jclass gatewayCLass = env->FindClass(WILTON_JNI_GATEWAY_INTERFACE);
    if (nullptr == gatewayCLass) { return -1; }
    GATEWAY_INTERFACE_CLASS = reinterpret_cast<jclass> (env->NewGlobalRef(gatewayCLass));
    if (nullptr == GATEWAY_INTERFACE_CLASS) { return -1; }
    GATEWAY_CALLBACK_METHOD = env->GetMethodID(GATEWAY_INTERFACE_CLASS, "gatewayCallback", "(J)V");
    if (nullptr == GATEWAY_CALLBACK_METHOD) { return -1; }
    // file
    jclass fileClass = env->FindClass("java/io/File");
    if (nullptr == fileClass) { return -1; }
    FILE_CLASS = reinterpret_cast<jclass> (env->NewGlobalRef(fileClass));
    if (nullptr == FILE_CLASS) { return -1; }
    FILE_CONSTRUCTOR_METHOD = env->GetMethodID(FILE_CLASS, "<init>", "(Ljava/lang/String;)V");
    if (nullptr == FILE_CONSTRUCTOR_METHOD) { return -1; }
    FILE_DELETE_METHOD = env->GetMethodID(FILE_CLASS, "delete", "()Z");
    if (nullptr == FILE_DELETE_METHOD) { return -1; }    
    return JNI_VERSION_1_6;
}

// todo: GlobalRef for gateway
JNIEXPORT jlong JNICALL WILTON_JNI_FUNCTION(createServer)
(JNIEnv* env, jclass, jobject gateway, jstring conf) {
    if (nullptr == gateway) { throwException(env, "Null 'gateway' parameter specified"); return 0; }
    if (nullptr == conf) { throwException(env, "Null 'conf' parameter specified"); return 0; }
    // todo: fixme - delete
    gateway = env->NewGlobalRef(gateway);
    
    wilton_Server* server_impl;
    const char* conf_cstr = env->GetStringUTFChars(conf, 0);
    int conf_len = static_cast<int> (env->GetStringUTFLength(conf));
    char* err = wilton_Server_create(std::addressof(server_impl),
            gateway,
            [](void* gateway_ctx, wilton_Request* request) {
                jobject gateway = static_cast<jobject>(gateway_ctx);
                jlong requestHandle = reinterpret_cast<jlong> (request);
                callGateway(gateway, requestHandle);
            }, conf_cstr, conf_len);
    env->ReleaseStringUTFChars(conf, conf_cstr);

    if (nullptr == err) {
        jlong handle = reinterpret_cast<jlong> (server_impl);
        return handle;
    } else {
        throwException(env, err);
        wilton_free(err);
        return 0;
    }
}

JNIEXPORT void JNICALL WILTON_JNI_FUNCTION(stopServer)
(JNIEnv* env, jclass, jlong serverHandle) {
    wilton_Server* server = serverFromHandle(env, serverHandle);
    if (nullptr == server) { return; }
    char* err = wilton_Server_stop_server(server);
    if (nullptr != err) {
        throwException(env, err);
        wilton_free(err);
    }
}

jstring JNICALL WILTON_JNI_FUNCTION(getRequestMetadata)
(JNIEnv* env, jclass, jlong requestHandle) {
    wilton_Request* request = requestFromHandle(env, requestHandle);
    if (nullptr == request) { return nullptr; }
    char* metadata;
    int metadata_len;
    char* err = wilton_Request_get_request_metadata(request, 
            std::addressof(metadata), std::addressof(metadata_len));    
    if (nullptr == err) {
        // consider it nul-terminated
        jstring res = env->NewStringUTF(metadata);
        wilton_free(metadata);
        return res;
    } else {
        throwException(env, err);
        wilton_free(err);
        return nullptr;
    }
}

jstring JNICALL WILTON_JNI_FUNCTION(getRequestData)
(JNIEnv* env, jclass, jlong requestHandle) {
    wilton_Request* request = requestFromHandle(env, requestHandle);
    if (nullptr == request) { return nullptr; }
    char* data;
    int data_len;
    char* err = wilton_Request_get_request_data(request,
            std::addressof(data), std::addressof(data_len));
    if (nullptr == err) {
        // consider it nul-terminated
        jstring res = env->NewStringUTF(data);
        wilton_free(data);
        return res;
    } else {
        throwException(env, err);
        wilton_free(err);
        return nullptr;
    }
}

void JNICALL WILTON_JNI_FUNCTION(setResponseMetadata)
(JNIEnv* env, jclass, jlong requestHandle, jstring conf) {
    if (nullptr == conf) { throwException(env, "Null 'conf' parameter specified"); return; }
    wilton_Request* request = requestFromHandle(env, requestHandle);
    if (nullptr == request) { return; }
    const char* conf_cstr = env->GetStringUTFChars(conf, 0);
    int conf_len = static_cast<int>(env->GetStringUTFLength(conf));
    char* err = wilton_Request_set_response_metadata(request, conf_cstr, conf_len);
    env->ReleaseStringUTFChars(conf, conf_cstr);
    if (nullptr != err) {
        throwException(env, err);
        wilton_free(err);
    }
}

JNIEXPORT void JNICALL WILTON_JNI_FUNCTION(sendResponse)
(JNIEnv* env, jclass, jlong requestHandle, jstring data) {
    if (nullptr == data) { throwException(env, "Null 'data' parameter specified"); return; }
    wilton_Request* request = requestFromHandle(env, requestHandle);
    if (nullptr == request) { return; }
    const char* data_cstr = env->GetStringUTFChars(data, 0);
    int data_len = static_cast<int> (env->GetStringUTFLength(data));
    char* err = wilton_Request_send_response(request, data_cstr, data_len);
    env->ReleaseStringUTFChars(data, data_cstr);
    if (nullptr != err) {
        throwException(env, err);
        wilton_free(err);
    }
}

JNIEXPORT void JNICALL WILTON_JNI_FUNCTION(sendTempFile)
(JNIEnv* env, jclass, jlong requestHandle, jstring file_path) {
    if (nullptr == file_path) { throwException(env, "Null 'file_path' parameter specified"); return; } 
    wilton_Request* request = requestFromHandle(env, requestHandle);
    if (nullptr == request) { return; }
    file_path = reinterpret_cast<jstring>(env->NewGlobalRef(file_path));
    const char* file_path_cstr = env->GetStringUTFChars(file_path, 0);
    int file_path_len = static_cast<int> (env->GetStringUTFLength(file_path));    
    char* err = wilton_Request_send_file(request, file_path_cstr, file_path_len, file_path, 
            [](void* ctx, int) {
                jstring file_path_passed = reinterpret_cast<jstring>(ctx);
                deleteFile(file_path_passed);                
            });    
    env->ReleaseStringUTFChars(file_path, file_path_cstr);
    if (nullptr != err) {
        throwException(env, err);
        wilton_free(err);
    }
}

JNIEXPORT void JNICALL WILTON_JNI_FUNCTION(sendMustache)
(JNIEnv* env, jclass, jlong requestHandle, jstring mustache_file_path, jstring values_json) {
    if (nullptr == mustache_file_path) { throwException(env, "Null 'mustache_file_path' parameter specified"); return; }
    if (nullptr == values_json) { throwException(env, "Null 'values_json' parameter specified"); return; }
    wilton_Request* request = requestFromHandle(env, requestHandle);
    if (nullptr == request) { return; }
    const char* mustache_file_path_cstr = env->GetStringUTFChars(mustache_file_path, 0);
    int mustache_file_path_len = static_cast<int> (env->GetStringUTFLength(mustache_file_path));
    const char* values_json_cstr = env->GetStringUTFChars(values_json, 0);
    int values_json_len = static_cast<int> (env->GetStringUTFLength(values_json));    
    char* err = wilton_Request_send_mustache(request, mustache_file_path_cstr, mustache_file_path_len, 
            values_json_cstr, values_json_len);
    env->ReleaseStringUTFChars(mustache_file_path, mustache_file_path_cstr);
    env->ReleaseStringUTFChars(values_json, values_json_cstr);
    if (nullptr != err) {
        throwException(env, err);
        wilton_free(err);
    }
}

JNIEXPORT void JNICALL WILTON_JNI_FUNCTION(appendLog)
(JNIEnv* env, jclass, jstring level, jstring logger, jstring message) {
    if (nullptr == level) { throwException(env, "Null 'level' parameter specified"); return; }    
    if (nullptr == logger) { throwException(env, "Null 'logger' parameter specified"); return; }    
    if (nullptr == message) { throwException(env, "Null 'message' parameter specified"); return; }
    const char* level_cstr = env->GetStringUTFChars(level, 0);
    int level_len = static_cast<int> (env->GetStringUTFLength(level));
    const char* logger_cstr = env->GetStringUTFChars(logger, 0);
    int logger_len = static_cast<int> (env->GetStringUTFLength(logger));
    const char* message_cstr = env->GetStringUTFChars(message, 0);
    int message_len = static_cast<int> (env->GetStringUTFLength(message));
    char* err = wilton_log(level_cstr, level_len, logger_cstr, logger_len, message_cstr, message_len);
    env->ReleaseStringUTFChars(level, level_cstr);
    env->ReleaseStringUTFChars(logger, logger_cstr);
    env->ReleaseStringUTFChars(message, message_cstr);
    if (nullptr != err) {
        throwException(env, err);
        wilton_free(err);
    }
}

JNIEXPORT jstring JNICALL WILTON_JNI_FUNCTION(renderMustache)
(JNIEnv* env, jclass, jstring templateText, jstring valuesJson) {
    if (nullptr == templateText) { throwException(env, "Null 'templateText' parameter specified"); return nullptr; }    
    if (nullptr == valuesJson) { throwException(env, "Null 'valuesJson' parameter specified"); return nullptr; }
    const char* templateText_cstr = env->GetStringUTFChars(templateText, 0);
    int templateText_len = static_cast<int> (env->GetStringUTFLength(templateText));
    const char* valuesJson_cstr = env->GetStringUTFChars(valuesJson, 0);
    int valuesJson_len = static_cast<int> (env->GetStringUTFLength(valuesJson));
    char* data;
    int data_len;
    char* err = wilton_render_mustache(templateText_cstr, templateText_len, valuesJson_cstr, valuesJson_len,
            std::addressof(data), std::addressof(data_len));
    env->ReleaseStringUTFChars(templateText, templateText_cstr);
    env->ReleaseStringUTFChars(valuesJson, valuesJson_cstr);
    if (nullptr == err) {
        // consider it nul-terminated
        jstring res = env->NewStringUTF(data);
        wilton_free(data);
        return res;
    } else {
        throwException(env, err);
        wilton_free(err);
        return nullptr;
    }
}

JNIEXPORT jlong JNICALL WILTON_JNI_FUNCTION(openDbConnection)
(JNIEnv* env, jclass, jstring url) {
    if (nullptr == url) { throwException(env, "Null 'url' parameter specified"); return 0; }
    wilton_DBConnection* conn_impl;
    const char* url_cstr = env->GetStringUTFChars(url, 0);
    int url_len = static_cast<int> (env->GetStringUTFLength(url));
    char* err = wilton_DBConnection_open(std::addressof(conn_impl), url_cstr, url_len);
    env->ReleaseStringUTFChars(url, url_cstr);
    if (nullptr == err) {
        jlong handle = reinterpret_cast<jlong> (conn_impl);
        return handle;
    } else {
        throwException(env, err);
        wilton_free(err);
        return 0;
    }
}

JNIEXPORT jstring JNICALL WILTON_JNI_FUNCTION(dbQuery)
(JNIEnv* env, jclass, jlong connectionHandle, jstring sql, jstring paramsJson) {
    if (nullptr == sql) { throwException(env, "Null 'sql' parameter specified"); return nullptr; }
    if (nullptr == paramsJson) { throwException(env, "Null 'paramsJson' parameter specified"); return nullptr; }
    wilton_DBConnection* conn = connectionFromHandle(env, connectionHandle);
    const char* sql_cstr = env->GetStringUTFChars(sql, 0);
    int sql_len = static_cast<int> (env->GetStringUTFLength(sql));
    const char* paramsJson_cstr = env->GetStringUTFChars(paramsJson, 0);
    int paramsJson_len = static_cast<int> (env->GetStringUTFLength(paramsJson));
    char* data;
    int data_len;
    char* err = wilton_DBConnection_query(conn, sql_cstr, sql_len, paramsJson_cstr, paramsJson_len, 
            std::addressof(data), std::addressof(data_len));
    env->ReleaseStringUTFChars(sql, sql_cstr);
    env->ReleaseStringUTFChars(paramsJson, paramsJson_cstr);
    if (nullptr == err) {
        // consider it nul-terminated
        jstring res = env->NewStringUTF(data);
        wilton_free(data);
        return res;
    } else {
        throwException(env, err);
        wilton_free(err);
        return nullptr;
    }
}

JNIEXPORT void JNICALL WILTON_JNI_FUNCTION(dbExecute)
(JNIEnv* env, jclass, jlong connectionHandle, jstring sql, jstring paramsJson) {
    if (nullptr == sql) { throwException(env, "Null 'sql' parameter specified"); return; }
    if (nullptr == paramsJson) { throwException(env, "Null 'paramsJson' parameter specified"); return; }
    wilton_DBConnection* conn = connectionFromHandle(env, connectionHandle);
    const char* sql_cstr = env->GetStringUTFChars(sql, 0);
    int sql_len = static_cast<int> (env->GetStringUTFLength(sql));
    const char* paramsJson_cstr = env->GetStringUTFChars(paramsJson, 0);
    int paramsJson_len = static_cast<int> (env->GetStringUTFLength(paramsJson));
    char* err = wilton_DBConnection_execute(conn, sql_cstr, sql_len, paramsJson_cstr, paramsJson_len);
    env->ReleaseStringUTFChars(sql, sql_cstr);
    env->ReleaseStringUTFChars(paramsJson, paramsJson_cstr);
    if (nullptr != err) {
        throwException(env, err);
        wilton_free(err);
    }
}

JNIEXPORT void JNICALL WILTON_JNI_FUNCTION(closeDbConnection)
(JNIEnv* env, jclass, jlong connectionHandle) {
    wilton_DBConnection* conn = connectionFromHandle(env, connectionHandle);
    char* err = wilton_DBConnection_close(conn);
    if (nullptr != err) {
        throwException(env, err);
        wilton_free(err);
    }
}

JNIEXPORT jlong JNICALL WILTON_JNI_FUNCTION(startDbTransaction)
(JNIEnv* env, jclass, jlong connectionHandle) {
    wilton_DBConnection* conn = connectionFromHandle(env, connectionHandle);
    wilton_DBTransaction* tran_impl;
    char* err = wilton_DBTransaction_start(conn, std::addressof(tran_impl));
    if (nullptr == err) {
        jlong handle = reinterpret_cast<jlong> (tran_impl);
        return handle;
    } else {
        throwException(env, err);
        wilton_free(err);
        return 0;
    }
}

JNIEXPORT void JNICALL WILTON_JNI_FUNCTION(commitDbTransaction)
(JNIEnv* env, jclass, jlong transactionHandle) {
    wilton_DBTransaction* tran = transactionFromHandle(env, transactionHandle);
    char* err = wilton_DBTransaction_commit(tran);
    if (nullptr != err) {
        throwException(env, err);
        wilton_free(err);
    }
}

JNIEXPORT void JNICALL WILTON_JNI_FUNCTION(rollbackDbTransaction)
(JNIEnv* env, jclass, jlong transactionHandle) {
    wilton_DBTransaction* tran = transactionFromHandle(env, transactionHandle);
    char* err = wilton_DBTransaction_rollback(tran);
    if (nullptr != err) {
        throwException(env, err);
        wilton_free(err);
    }
}


} // C
