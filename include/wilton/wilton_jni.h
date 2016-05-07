/* 
 * File:   wilton_jni.h
 * Author: alex
 *
 * Created on May 1, 2016, 12:07 AM
 */

#ifndef WILTON_JNI_H
#define	WILTON_JNI_H

#include "jni.h"

// todo: compile time check
#define WILTON_JNI_CLASS WiltonJniTest
#define WILTON_JNI_GATEWAY_INTERFACE WiltonJniTestGateway

#define WILTON_JNI_PASTER(x,y) Java_ ## x ## _ ## y
#define WILTON_JNI_EVALUATOR(x,y) WILTON_JNI_PASTER(x,y)
#define WILTON_JNI_FUNCTION(fun) WILTON_JNI_EVALUATOR(WILTON_JNI_CLASS, fun)

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*
 * Class:     WILTON_JNI_CLASS
 * Method:    createServer
 * Signature: (Ljava/lang/Object;Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL WILTON_JNI_FUNCTION(createServer)
(JNIEnv *, jclass, jobject, jstring);

/*
 * Class:     WILTON_JNI_CLASS
 * Method:    stopServer
 * Signature: (J)V
 */
JNIEXPORT void JNICALL WILTON_JNI_FUNCTION(stopServer)
(JNIEnv *, jclass, jlong);

/*
 * Class:     WILTON_JNI_CLASS
 * Method:    getRequestMetadata
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL WILTON_JNI_FUNCTION(getRequestMetadata)
(JNIEnv *, jclass, jlong);

/*
 * Class:     WILTON_JNI_CLASS
 * Method:    getRequestData
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL WILTON_JNI_FUNCTION(getRequestData)
(JNIEnv *, jclass, jlong);

/*
 * Class:     WILTON_JNI_CLASS
 * Method:    setResponseMetadata
 * Signature: (JLjava/lang/String;)V
 */
JNIEXPORT void JNICALL WILTON_JNI_FUNCTION(setResponseMetadata)
(JNIEnv *, jclass, jlong, jstring);

/*
 * Class:     WILTON_JNI_CLASS
 * Method:    sendResponse
 * Signature: (JLjava/lang/String;)V
 */
JNIEXPORT void JNICALL WILTON_JNI_FUNCTION(sendResponse)
(JNIEnv *, jclass, jlong, jstring);

/* TODO
 * Class:     WILTON_JNI_CLASS
 * Method:    sendResponseChunked
 * Signature: (JJLjava/io/InputStream;)V
 
JNIEXPORT void JNICALL WILTON_JNI_FUNCTION(sendResponseChunked)
(JNIEnv *, jclass, jlong, jlong, jobject);
 */

#ifdef __cplusplus
}
#endif // __cplusplus

#endif	/* WILTON_JNI_H */

