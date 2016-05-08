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

JavaVM* JAVA_VM; 

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

// TODO: error reporting 
//    if (nullptr == clazz) { throwException(env, TRACEMSG(std::string() + "Gateway interface not found: [" + WILTON_JNI_GATEWAY_INTERFACE + "]").c_str()); }
//    if (nullptr == method) { throwException(env, TRACEMSG(std::string() + "Gateway callback method not found: [gatewayCallback]").c_str()); }
void callGateway(jobject gateway, jlong requestHandle) {
    JNIEnv* env;
    auto err = JAVA_VM->AttachCurrentThread(std::addressof(env), nullptr);
    if (JNI_OK != err) { return; } // cannot report error here
    jclass clazz = env->FindClass(WILTON_JNI_GATEWAY_INTERFACE);
    if (nullptr != clazz) { 
        jmethodID method = env->GetMethodID(clazz, "gatewayCallback", "(J)V");
        if (nullptr != method) {
            env->CallVoidMethod(gateway, method, requestHandle);
            jthrowable exc = env->ExceptionOccurred();
            if (nullptr != exc) {
                env->ExceptionClear();
            }
        }
    }
    JAVA_VM->DetachCurrentThread();
}

} // namespace

extern "C" {

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void*) {    
    JNIEnv* env;
    auto err = vm->GetEnv(reinterpret_cast<void**> (std::addressof(env)), JNI_VERSION_1_6);
    if (JNI_OK != err) { return -1; }
    
    JAVA_VM = vm;
    
    // Get jclass with env->FindClass.
    // Register methods with env->RegisterNatives.
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

} // C
