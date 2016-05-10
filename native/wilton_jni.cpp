/* 
 * File:   wilton_jni.cpp
 * Author: alex
 *
 * Created on May 1, 2016, 12:08 AM
 */


#include "jni.h"

#include <string>

#include "staticlib/config.hpp"

#include "wilton/wilton_c.h"

// todo: compile time check
#define WILTON_JNI_CLASS net_wiltonwebtoolkit_HttpServerJni
#define WILTON_JNI_GATEWAY_INTERFACE "net/wiltonwebtoolkit/HttpGateway"
#define WILTON_JNI_EXCEPTION_CLASS "net/wiltonwebtoolkit/HttpException"

#define WILTON_JNI_PASTER(x,y) Java_ ## x ## _ ## y
#define WILTON_JNI_EVALUATOR(x,y) WILTON_JNI_PASTER(x,y)
#define WILTON_JNI_FUNCTION(fun) WILTON_JNI_EVALUATOR(WILTON_JNI_CLASS, fun)

namespace { // namespace

namespace sc = staticlib::config;

// globals

JavaVM* JAVA_VM;
jclass GATEWAY_INTERFACE_CLASS;
jmethodID GATEWAY_CALLBACK_METHOD;


void throwException(JNIEnv* env, const char* message) {
    jclass exClass = env->FindClass(WILTON_JNI_EXCEPTION_CLASS);
    std::string msg = TRACEMSG(std::string() + message + "\nC API Error");
    env->ThrowNew(exClass, msg.c_str());
}

wilton_Server* serverFromHandle(JNIEnv*, jlong handle) {
    return reinterpret_cast<wilton_Server*> (handle);
}

wilton_Request* requestFromHandle(JNIEnv*, jlong requestHandle) {
    return reinterpret_cast<wilton_Request*> (requestHandle);
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

} // namespace

extern "C" {

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void*) {    
    JNIEnv* env;
    auto err = vm->GetEnv(reinterpret_cast<void**>(std::addressof(env)), JNI_VERSION_1_6);
    if (JNI_OK != err) { return -1; }
    JAVA_VM = vm;
    GATEWAY_INTERFACE_CLASS = env->FindClass(WILTON_JNI_GATEWAY_INTERFACE);
    if (nullptr == GATEWAY_INTERFACE_CLASS) { return -1; }
    GATEWAY_CALLBACK_METHOD = env->GetMethodID(GATEWAY_INTERFACE_CLASS, "gatewayCallback", "(J)V");
    if (nullptr == GATEWAY_CALLBACK_METHOD) { return -1; }
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

JNIEXPORT void JNICALL WILTON_JNI_FUNCTION(appendLog)
(JNIEnv* env, jclass, jint level, jstring logger, jstring message) {
    (void) env;
    (void) level;
    (void) logger;
    (void) message;
}

} // C
