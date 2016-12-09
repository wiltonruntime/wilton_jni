/* 
 * File:   wiltonjs_thread.cpp
 * Author: alex
 *
 * Created on December 9, 2016, 10:31 PM
 */

#include "wiltonjs/wiltonjs.hpp"

#include <functional>

#include "jni.h"

#include "wilton/wilton.h"

namespace wiltonjs {

namespace { // anonymous

namespace ss = staticlib::serialization;

const std::string EMPTY_STRING = "";

void call_runnable(JNIEnv* env, jobject runnable) {
    try {
        env->CallVoidMethod(runnable, static_cast<jmethodID> (detail::get_runnable_method()));
        if (env->ExceptionOccurred()) {
            // todo: details
            detail::log_error(TRACEMSG("Tread runnable Java exception caught, ignoring"));
            env->ExceptionClear();
        }
    } catch (const std::exception& e) {
        detail::log_error(TRACEMSG(e.what()));
    }
}

} // namespace

std::string thread_run(const std::string&, void* object) {
    JNIEnv* env = static_cast<JNIEnv*> (detail::get_jni_env());
    jobject runnable_local = static_cast<jobject> (object);
    jobject runnable = env->NewGlobalRef(runnable_local);
    // call wilton
    char* err = wilton_thread_run(runnable,
            [](void* passed) {
                JNIEnv* env = static_cast<JNIEnv*> (detail::get_jni_env());
                jobject runnable_passed = static_cast<jobject> (passed);
                        call_runnable(env, runnable_passed);
                        env->DeleteGlobalRef(runnable_passed);
            });
    if (nullptr != err) {
        detail::throw_wilton_error(err, TRACEMSG(std::string(err)));
    }
    return "{}";
}

std::string thread_sleep_millis(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t millis = -1;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("millis" == name) {
            millis = detail::get_json_int(fi);
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == millis) throw WiltonJsException(TRACEMSG(
            "Required parameter 'millis' not specified"));
    // call wilton
    char* err = wilton_thread_sleep_millis(static_cast<int> (millis));
    if (nullptr != err) {
        detail::throw_wilton_error(err, TRACEMSG(std::string(err)));
    }
    return "{}";
}

} // namespace



