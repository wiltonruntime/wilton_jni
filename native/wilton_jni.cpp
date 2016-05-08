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
#include "staticlib/utils.hpp"

#include "wilton/wilton_c.h"

// todo: compile time check
#define WILTON_JNI_CLASS net_wiltonwebtoolkit_HttpServerJni
#define WILTON_JNI_GATEWAY_INTERFACE net/wiltonwebtoolkit/HttpGateway

#define WILTON_JNI_PASTER(x,y) Java_ ## x ## _ ## y
#define WILTON_JNI_EVALUATOR(x,y) WILTON_JNI_PASTER(x,y)
#define WILTON_JNI_FUNCTION(fun) WILTON_JNI_EVALUATOR(WILTON_JNI_CLASS, fun)

namespace { // namespace

namespace sc = staticlib::config;
namespace su = staticlib::utils;

JavaVM* JAVA_VM; 

void throwException(JNIEnv* env, char* message) {
    jclass exClass = env->FindClass("java/lang/RuntimeException");
    env->ThrowNew(exClass, TRACEMSG(std::string() + message + "\nC API Error");
//    wilton_free_errmsg(message);
}

wilton_Server* serverFromHandle(JNIEnv* env, jlong handle) {
    return reinterpret_cast<wilton_Server*> (handle);
}

wilton_Request* requestFromHandle(JNIEnv* env, jlong requestHandle) {
    return reinterpret_cast<wilton_Request*> (handle);
}

void callGateway(jobject gateway, jlong requestHandle) {
    JNIEnv* env;
    auto err = vm->AttachCurrentThread(reinterpret_cast<void**>(std::addressof(env)), nullptr);
    if (JNI_OK != err) { return; } // cannot report error here
    jclass clazz = env->FindClass(WILTON_JNI_GATEWAY_INTERFACE);
    if (nullptr == clazz) { throwException(TRACEMSG("Gateway interface not found: [" + WILTON_JNI_GATEWAY_INTERFACE + "]")) }
    jmethodID method = env->GetMethodID(clazz, "gatewayCallback", "(J)V");
    // todo: report calss name
    if (nullptr == clazz) { throwException(TRACEMSG("Gateway callback method not found: [gatewayCallback]")) }
    env->CallVoidMethod(gateway, method, requestHandle);
    vm->DetachCurrentThread();
}

} // namespace

extern "C" {

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {    
    JNIEnv* env;
    auto err = vm->GetEnv(reinterpret_cast<void**> (std::addressof(env)), JNI_VERSION_1_6);
    if (JNI_OK != err) { return -1; }
    
    JAVA_VM = vm;
    
    // Get jclass with env->FindClass.
    // Register methods with env->RegisterNatives.
    return JNI_VERSION_1_6;
}

// TODO: attach thread
// TODO: exception in cb
JNIEXPORT jlong JNICALL WILTON_JNI_FUNCTION(createServer)
(JNIEnv* env, jclass, jobject gateway, jstring conf) {
    if (nullptr == gateway) { throwWiltonExc(env, "Null 'gateway' parameter specified"); return; }
    if (nullptr == conf) { throwWiltonExc(env, "Null 'conf' parameter specified"); return; }
    
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
        JniServer* server = new JniServer(server_impl);
        jlong handle = reinterpret_cast<jlong> (server);
        VALID_SERVER_HANDLES.insert(handle);
        return handle;
    } else {
        throwException(env, err);
        wilton_free_errmsg(err);
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
