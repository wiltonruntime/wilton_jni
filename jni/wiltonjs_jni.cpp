/* 
 * File:   wiltonjs_jni.cpp
 * Author: alex
 *
 * Created on May 1, 2016, 12:08 AM
 */


#include "jni.h"

#include <atomic>
#include <memory>
#include <string>

#include "staticlib/io.hpp"
#include "staticlib/utils.hpp"

#include "wilton/wilton.h"

#include "wiltonjs/wiltoncall.hpp"
#include "wiltonjs/wiltonjs.hpp"

#define WILTON_JNI_CLASS net_wiltonwebtoolkit_WiltonJni
#define WILTON_JNI_GATEWAY_INTERFACE "net/wiltonwebtoolkit/WiltonGateway"
#define WILTON_JNI_EXCEPTION_CLASS "net/wiltonwebtoolkit/WiltonException"
#define WILTON_STARTUP_ERR_DIR ""

#define WILTON_JNI_PASTER(x,y) Java_ ## x ## _ ## y
#define WILTON_JNI_EVALUATOR(x,y) WILTON_JNI_PASTER(x,y)
#define WILTON_JNI_FUNCTION(fun) WILTON_JNI_EVALUATOR(WILTON_JNI_CLASS, fun)

namespace { // anonymous

namespace si = staticlib::io;
namespace su = staticlib::utils;
namespace wj = wiltonjs;

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

// shouldn't be called before logging is initialized by app

void log_error(const std::string& message) {
    std::string level = "ERROR";
    std::string logger = "wilton.jni";
    // call wilton
    wilton_logger_log(level.c_str(), level.length(), logger.c_str(), logger.length(),
            message.c_str(), message.length());
}

std::string jstring_to_str(JNIEnv* env, jstring jstr) {
    const char* cstr = env->GetStringUTFChars(jstr, 0);
    size_t cstr_len = static_cast<size_t> (env->GetStringUTFLength(jstr));
    std::string res{cstr, cstr_len};
    env->ReleaseStringUTFChars(jstr, cstr);
    return res;
}

std::atomic<bool>& static_jvm_active() {
    static std::atomic<bool> flag{false};
    return flag;
}

void* /* JNIEnv* */ get_jni_env();


class GlobalRefDeleter {
public:
    void operator()(jobject ref) {
        if (static_jvm_active().load()) {
            JNIEnv* env = static_cast<JNIEnv*> (get_jni_env());
            env->DeleteGlobalRef(ref);
        }
    }
};

std::unique_ptr<_jclass, GlobalRefDeleter> find_java_class(JNIEnv* env, const std::string& name) {
    jclass local = env->FindClass(name.c_str());
    if (nullptr == local) {
        throw wiltonjs::WiltonJsException(TRACEMSG("Cannot load class, name: [" + name + "]"));
    }
    std::unique_ptr<_jclass, GlobalRefDeleter> res{static_cast<jclass>(env->NewGlobalRef(local)), GlobalRefDeleter()};
    if (nullptr == res.get()) {
        throw wiltonjs::WiltonJsException(TRACEMSG("Cannot create global ref for class, name: [" + name + "]"));
    }
    env->DeleteLocalRef(local);
    return res;
}

jmethodID find_java_method(JNIEnv* env, jclass clazz, const std::string& name, const std::string& signature) {
    jmethodID res = env->GetMethodID(clazz, name.c_str(), signature.c_str());
    if (nullptr == res) {
        throw wiltonjs::WiltonJsException(TRACEMSG("Cannot find method, name: [" + name + "]," +
                " signature: [" + signature + "]"));
    }
    return res;
}


class JniCtx {
public:    
    JavaVM* vm;
    std::unique_ptr<_jclass, GlobalRefDeleter> gatewayInterface;
    jmethodID gatewayCallbackMethod;
    std::unique_ptr<_jclass, GlobalRefDeleter> callableInterface;
    jmethodID callableMethod;
    std::unique_ptr<_jclass, GlobalRefDeleter> wiltonExceptionClass;    

    JniCtx(const JniCtx&) = delete;
    
    JniCtx& operator=(const JniCtx&) = delete;

    JniCtx(JniCtx&&) = delete;

    JniCtx& operator=(JniCtx&& other) {
        this->vm = other.vm;
        other.vm = nullptr;
        this->gatewayInterface = std::move(other.gatewayInterface);
        this->gatewayCallbackMethod = other.gatewayCallbackMethod;
        other.gatewayCallbackMethod = nullptr;
        this->callableInterface = std::move(other.callableInterface);
        this->callableMethod = other.callableMethod;
        other.callableMethod = nullptr;
        this->wiltonExceptionClass = std::move(other.wiltonExceptionClass);
        return *this;
    }
    
    JniCtx() { }
    
    JniCtx(JavaVM* vm) :
    vm(vm) {
        // env
        JNIEnv* env;
        auto err = vm->GetEnv(reinterpret_cast<void**> (std::addressof(env)), JNI_VERSION_1_6);
        if (JNI_OK != err) {
            throw wiltonjs::WiltonJsException(TRACEMSG("Cannot obtain JNI environment"));
        }
        // gateway
        this->gatewayInterface = find_java_class(env, WILTON_JNI_GATEWAY_INTERFACE);
        this->gatewayCallbackMethod = find_java_method(env, this->gatewayInterface.get(), 
                "gatewayCallback", "(Ljava/lang/String;J)V");
        // callable
        this->callableInterface = find_java_class(env, "java/util/concurrent/Callable");
        this->callableMethod = find_java_method(env, this->callableInterface.get(), 
                "call", "()Ljava/lang/Object;");
        // exception
        this->wiltonExceptionClass = find_java_class(env, WILTON_JNI_EXCEPTION_CLASS);
    }
};

JniCtx& static_jni_ctx() {
    static JniCtx* ctx = new JniCtx();
    return *ctx;
}

void dump_startup_error(const std::string& msg) {
    try {
        // err file setup
        auto errdir = std::string(WILTON_STARTUP_ERR_DIR);
        // random postfix
        std::string id = su::RandomStringGenerator().generate(12);
        auto errfile = errdir + "wilton_ERROR_" + id + ".txt";
        auto fd = su::FileDescriptor(errfile, 'w');
        si::write_all(fd, msg);
    } catch(...) {
        // give up
    }
}

void* /* JNIEnv* */ get_jni_env() {
    JavaVM* vm = static_jni_ctx().vm;
    JNIEnv* env;
    auto getenv_err = vm->GetEnv(reinterpret_cast<void**> (std::addressof(env)), JNI_VERSION_1_6);
    switch (getenv_err) {
    case JNI_OK:
        return static_cast<void*> (env);
    case JNI_EDETACHED:
        if (JNI_OK == vm->AttachCurrentThread(std::addressof(env), nullptr)) {
            return static_cast<void*> (env);
        }
        // fall-through to report error to client
    default:
        throw wj::WiltonJsException(TRACEMSG("System error: cannot obtain JNI environment"));
    }
}

} // namespace

// detail helpers

namespace wiltonjs {
namespace detail {

// todo: exceptions
std::string invoke_js_callable(void* callable, bool suppress_js_exception) {
    JNIEnv* env = static_cast<JNIEnv*> (get_jni_env());
    jobject obj = env->CallObjectMethod(static_cast<jobject> (callable), static_jni_ctx().callableMethod);
    if (env->ExceptionOccurred()) {
        if (suppress_js_exception) {
            log_error(TRACEMSG("Callable exception caught, ignoring"));
            env->ExceptionClear();
        }
        return "";
    }    
    return nullptr != obj ? jstring_to_str(env, static_cast<jstring> (obj)) : "";
}

void invoke_gateway(void* gateway, void* callbackModule, int64_t requestHandle) {
    JNIEnv* env = nullptr;
    try {
        env = static_cast<JNIEnv*> (get_jni_env());
        env->CallVoidMethod(static_cast<jobject>(gateway), static_jni_ctx().gatewayCallbackMethod,
                callbackModule, requestHandle);
        if (env->ExceptionOccurred()) {
            env->ExceptionDescribe();
            std::string msg = TRACEMSG("Gateway error");
            log_error(msg);
            // todo
            //send_system_error(requestHandle, msg);
            env->ExceptionClear();
        }
    } catch (const std::exception& e) {
        std::string msg = TRACEMSG(e.what() + "\nGateway error");
        log_error(msg);
        // todo
//        send_system_error(requestHandle, msg);
    }
}

// todo: delete
void* wrap_object_permanent(void* object) {
    JNIEnv* env = static_cast<JNIEnv*> (get_jni_env());
    return env->NewGlobalRef(static_cast<jobject>(object));
}

// todo: rework
void delete_wrapped_object(void* object) {
    if (static_jvm_active().load()) {
        JNIEnv* env = static_cast<JNIEnv*> (get_jni_env());
        return env->DeleteGlobalRef(static_cast<jobject> (object));
    }
}

void* /* jstring */ create_platform_string(const std::string& str) {
    JNIEnv* env = static_cast<JNIEnv*> (get_jni_env());
    return env->NewStringUTF(str.c_str());
}

void throw_js_exception(const std::string& message) {
    JNIEnv* env = static_cast<JNIEnv*>(get_jni_env());
    env->ThrowNew(static_jni_ctx().wiltonExceptionClass.get(),
            TRACEMSG(message + "\nReporting delayed error").c_str());
}

} // namespace
}

extern "C" {

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void*) {
    try {
        // wiltoncalls
        register_wiltoncalls();
        // move-assign static ctx
        static_jni_ctx() = JniCtx(vm);
        // set init flag
        static_jvm_active().store(true);
        return JNI_VERSION_1_6;
    } catch (const std::exception& e) {
        dump_startup_error(TRACEMSG(e.what() + "\nInitialization error"));
        return -1;
    }    
}

JNIEXPORT void JNI_OnUnload(JavaVM*, void*) {
    delete std::addressof(static_jni_ctx());
    // flip init flag
    static_jvm_active().store(false);
}

JNIEXPORT jstring JNICALL WILTON_JNI_FUNCTION(wiltoncall)
(JNIEnv* env, jclass, jstring name, jstring data, jobject object) {
    jclass exclass = static_jni_ctx().wiltonExceptionClass.get();
    if (nullptr == name) {
        env->ThrowNew(exclass, TRACEMSG("Null 'name' parameter specified").c_str());
        return nullptr;
    }
    if (nullptr == data) {
        env->ThrowNew(exclass, TRACEMSG("Null 'data' parameter specified").c_str());
        return nullptr;
    }
    std::string name_string = jstring_to_str(env, name);
    std::string data_string = jstring_to_str(env, data);
    try {
        std::string res = wj::invoke_wilton_function(name_string, data_string, static_cast<void*>(object));
        return env->NewStringUTF(res.c_str());
    } catch (const std::exception& e) {
        env->ThrowNew(exclass, TRACEMSG(e.what() + 
                "\n'wiltoncall' error for name: [" + name_string + "], data: [" + data_string + "]").c_str());
    } catch (...) {
        env->ThrowNew(exclass, TRACEMSG(
                "'wiltoncall' error for name: [" + name_string + "], data: [" + data_string + "]").c_str());
    }
    return nullptr;
}

} // C
