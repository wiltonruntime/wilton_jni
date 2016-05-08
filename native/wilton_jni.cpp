/* 
 * File:   wilton_jni.cpp
 * Author: alex
 *
 * Created on May 1, 2016, 12:08 AM
 */


#include "jni.h"

#include <cstring>

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

void callGateway(jobject gateway, jlong requestHandle) {
    JNIEnv* env;
    auto err = JAVA_VM->AttachCurrentThread(std::addressof(env), nullptr);
    if (JNI_OK != err) { return; } // cannot report error here
    jclass clazz = env->FindClass(WILTON_JNI_GATEWAY_INTERFACE);
    if (nullptr == clazz) { throwException(env, TRACEMSG(std::string() + "Gateway interface not found: [" + WILTON_JNI_GATEWAY_INTERFACE + "]").c_str()); }
    jmethodID method = env->GetMethodID(clazz, "gatewayCallback", "(J)V");
    // todo: report class name
    if (nullptr == clazz) { throwException(env, TRACEMSG(std::string() + "Gateway callback method not found: [gatewayCallback]").c_str()); }
    env->CallVoidMethod(gateway, method, requestHandle);
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

JNIEXPORT jlong JNICALL WILTON_JNI_FUNCTION(createServer)
(JNIEnv* env, jclass, jobject gateway, jstring conf) {
    if (nullptr == gateway) { throwException(env, "Null 'gateway' parameter specified"); return 0; }
    if (nullptr == conf) { throwException(env, "Null 'conf' parameter specified"); return 0; }
    
    wilton_Server* server_impl;
    const char* conf_cstr = env->GetStringUTFChars(conf, 0);
    char* err = wilton_Server_create(std::addressof(server_impl),
            gateway,
            [](void* gateway_ctx, wilton_Request* request) {
                jobject gateway = static_cast<jobject>(gateway_ctx);
                jlong requestHandle = reinterpret_cast<jlong> (request);
                callGateway(gateway, requestHandle);
            }, conf_cstr, strlen(conf_cstr));
    env->ReleaseStringUTFChars(conf, conf_cstr);

    if (nullptr == err) {
        jlong handle = reinterpret_cast<jlong> (server_impl);
        return handle;
    } else {
        throwException(env, err);
        wilton_free_errmsg(err);
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
        wilton_free_errmsg(err);
    }
}

/*
jstring JNICALL WILTON_JNI_FUNCTION(getRequestMetadata)
(JNIEnv* env, jclass, jlong requestHandle) {
    wilton_Request* request = requestFromHandle(env, requestHandle);
    if (nullptr == request) { return; }    
    char* err = wilton_Server_stop_server(server);
    if (nullptr != err) {
        throwException(env, err);
    }
}

jstring JNICALL WILTON_JNI_FUNCTION(getRequestData)
(JNIEnv* env, jclass, jlong requestHandle) {
    
}
 * 

void JNICALL WILTON_JNI_FUNCTION(setResponseMetadata)
(JNIEnv* env, jclass, jlong requestHandle, jstring conf) {
    
}
 * */

JNIEXPORT void JNICALL WILTON_JNI_FUNCTION(sendResponse)
(JNIEnv* env, jclass, jlong requestHandle, jstring data) {
    wilton_Request* request = requestFromHandle(env, requestHandle);
    if (nullptr == request) { return; }
    const char* data_cstr = env->GetStringUTFChars(data, 0);
    char* err = wilton_Request_send_response(request, data_cstr, strlen(data_cstr));
    env->ReleaseStringUTFChars(data, data_cstr);
    if (nullptr != err) {
        throwException(env, err);
        wilton_free_errmsg(err);
    }
}

} // C
