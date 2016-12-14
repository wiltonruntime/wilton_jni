/* 
 * File:   wiltonjs_jni.cpp
 * Author: alex
 *
 * Created on May 1, 2016, 12:08 AM
 */


#include "jni.h"

#include <string>

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

namespace wj = wiltonjs;

// globals
JavaVM* JAVA_VM;
jclass GATEWAY_INTERFACE_CLASS;
jmethodID GATEWAY_CALLBACK_METHOD;
jclass RUNNABLE_INTERFACE_CLASS;
jmethodID RUNNABLE_CALLBACK_METHOD;
jclass CALLABLE_INTERFACE_CLASS;
jmethodID CALLABLE_CALLBACK_METHOD;
jclass EXCEPTION_CLASS;

std::string jstring_to_str(JNIEnv* env, jstring jstr) {
    const char* cstr = env->GetStringUTFChars(jstr, 0);
    size_t cstr_len = static_cast<size_t> (env->GetStringUTFLength(jstr));
    std::string res{cstr, cstr_len};
    env->ReleaseStringUTFChars(jstr, cstr);
    return res;
}

void register_wiltoncalls() {
    wj::put_wilton_function("mustache_render", wj::mustache_render);
    wj::put_wilton_function("mustache_render_file", wj::mustache_render_file);
    
    wj::put_wilton_function("httpclient_create", wj::httpclient_create);
    wj::put_wilton_function("httpclient_close", wj::httpclient_close);
    wj::put_wilton_function("httpclient_execute", wj::httpclient_execute);
    wj::put_wilton_function("httpclient_send_temp_file", wj::httpclient_send_temp_file);
    
    wj::put_wilton_function("logger_initialize", wj::logger_initialize);
    wj::put_wilton_function("logger_log", wj::logger_log);
    wj::put_wilton_function("logger_is_level_enabled", wj::logger_is_level_enabled);
    wj::put_wilton_function("logger_shutdown", wj::logger_shutdown);
    
    wj::put_wilton_function("db_connection_open", wj::db_connection_open);
    wj::put_wilton_function("db_connection_query", wj::db_connection_query);
    wj::put_wilton_function("db_connection_execute", wj::db_connection_execute);
    wj::put_wilton_function("db_connection_close", wj::db_connection_close);
    wj::put_wilton_function("db_transaction_start", wj::db_transaction_start);
    wj::put_wilton_function("db_transaction_commit", wj::db_transaction_commit);
    wj::put_wilton_function("db_transaction_rollback", wj::db_transaction_rollback);
    
    wj::put_wilton_function("server_create", wj::server_create);
    wj::put_wilton_function("server_stop", wj::server_stop);
    wj::put_wilton_function("request_get_metadata", wj::request_get_metadata);
    wj::put_wilton_function("request_get_data", wj::request_get_data);
    wj::put_wilton_function("request_get_data_filename", wj::request_get_data_filename);
    wj::put_wilton_function("request_set_response_metadata", wj::request_set_response_metadata);
    wj::put_wilton_function("request_send_response", wj::request_send_response);
    wj::put_wilton_function("request_send_temp_file", wj::request_send_temp_file);
    wj::put_wilton_function("request_send_mustache", wj::request_send_mustache);
    wj::put_wilton_function("request_send_later", wj::request_send_later);
    wj::put_wilton_function("request_send_with_response_writer", wj::request_send_with_response_writer);
    
    wj::put_wilton_function("cron_start", wj::cron_start);
    wj::put_wilton_function("cron_stop", wj::cron_stop);
    
    wj::put_wilton_function("mutex_create", wj::mutex_create);
    wj::put_wilton_function("mutex_lock", wj::mutex_lock);
    wj::put_wilton_function("mutex_unlock", wj::mutex_unlock);
    wj::put_wilton_function("mutex_wait", wj::mutex_wait);
    wj::put_wilton_function("mutex_notify_all", wj::mutex_notify_all);
    wj::put_wilton_function("mutex_destroy", wj::mutex_destroy);
    
    wj::put_wilton_function("shared_put", wj::shared_put);
    wj::put_wilton_function("shared_get", wj::shared_get);
    wj::put_wilton_function("shared_wait_change", wj::shared_wait_change);
    wj::put_wilton_function("shared_remove", wj::shared_remove);
    
    wj::put_wilton_function("thread_run", wj::thread_run);
    wj::put_wilton_function("thread_sleep_millis", wj::thread_sleep_millis);
    
    wj::put_wilton_function("tcp_wait_for_connection", wj::tcp_wait_for_connection);
}

} // namespace

// detail helpers

namespace wiltonjs {
namespace detail {

void* /* JNIEnv* */ get_jni_env() {
    JNIEnv* env;
    auto getenv_err = JAVA_VM->GetEnv(reinterpret_cast<void**> (std::addressof(env)), JNI_VERSION_1_6);
    switch (getenv_err) {
    case JNI_OK:
        return static_cast<void*> (env);
    case JNI_EDETACHED:
        if (JNI_OK == JAVA_VM->AttachCurrentThread(std::addressof(env), nullptr)) {
            return static_cast<void*> (env);
        }
        // fall-through to report error to client
    default:
        throw WiltonJsException(TRACEMSG("System error: cannot obtain JNI environment"));
    }
}

void* /* jmethodID */ get_gateway_method() {
    return static_cast<void*> (GATEWAY_CALLBACK_METHOD);
}

void* /* jmethodID */ get_runnable_method() {
    return static_cast<void*> (RUNNABLE_CALLBACK_METHOD);
}

void* /* jmethodID */ get_callable_method() {
    return static_cast<void*> (CALLABLE_CALLBACK_METHOD);
}

void throw_delayed(const std::string& message) {
    JNIEnv* env = static_cast<JNIEnv*>(get_jni_env());
    env->ThrowNew(EXCEPTION_CLASS, TRACEMSG(message + "\nReporting delayed error").c_str());
}

// shouldn't be called before logging is initialized by app
void log_error(const std::string& message) {
    std::string level = "ERROR";
    std::string logger = "wilton.jni";
    // call wilton
    wilton_logger_log(level.c_str(), level.length(), logger.c_str(), logger.length(),
            message.c_str(), message.length());
}

} // namespace
}

extern "C" {

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void*) {
    // wiltoncalls, will crash loudly on register error
    register_wiltoncalls();
    // env
    JNIEnv* env;
    auto err = vm->GetEnv(reinterpret_cast<void**>(std::addressof(env)), JNI_VERSION_1_6);
    if (JNI_OK != err) { return -1; }
    JAVA_VM = vm;
    // gateway
    jclass gatewayClass = env->FindClass(WILTON_JNI_GATEWAY_INTERFACE);
    if (nullptr == gatewayClass) { return -1; }
    GATEWAY_INTERFACE_CLASS = static_cast<jclass> (env->NewGlobalRef(gatewayClass));
    if (nullptr == GATEWAY_INTERFACE_CLASS) { return -1; }
    GATEWAY_CALLBACK_METHOD = env->GetMethodID(GATEWAY_INTERFACE_CLASS, "gatewayCallback", "(Ljava/lang/String;J)V");
    if (nullptr == GATEWAY_CALLBACK_METHOD) { return -1; }
    // runnable
    jclass runnableClass = env->FindClass("java/lang/Runnable");
    if (nullptr == runnableClass) { return -1; }
    RUNNABLE_INTERFACE_CLASS = static_cast<jclass> (env->NewGlobalRef(runnableClass));
    if (nullptr == RUNNABLE_INTERFACE_CLASS) {return -1; }
    RUNNABLE_CALLBACK_METHOD = env->GetMethodID(RUNNABLE_INTERFACE_CLASS, "run", "()V");
    if (nullptr == RUNNABLE_CALLBACK_METHOD) { return -1; }
    // callable
    jclass callableClass = env->FindClass("java/util/concurrent/Callable");
    if (nullptr == callableClass) { return -1; }
    CALLABLE_INTERFACE_CLASS = static_cast<jclass> (env->NewGlobalRef(callableClass));
    if (nullptr == CALLABLE_INTERFACE_CLASS) { return -1; }
    CALLABLE_CALLBACK_METHOD = env->GetMethodID(CALLABLE_INTERFACE_CLASS, "call", "()Ljava/lang/Object;");
    if (nullptr == CALLABLE_CALLBACK_METHOD) { return -1; }
    // exception
    jclass exClass = env->FindClass(WILTON_JNI_EXCEPTION_CLASS);
    if (nullptr == exClass) { return -1; }
    EXCEPTION_CLASS = static_cast<jclass> (env->NewGlobalRef(exClass));    
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
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG(e.what() + 
                "\nwiltoncall error for name: [" + name_string + "], data: [" + data_string + "]").c_str());
    } catch (...) {
        env->ThrowNew(EXCEPTION_CLASS, TRACEMSG(
                "\nwiltoncall error for name: [" + name_string + "], data: [" + data_string + "]").c_str());
    }
    return nullptr;
}

} // C
