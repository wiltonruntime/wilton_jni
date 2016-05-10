/*
 * Copyright 2016, alex at staticlibs.net
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package net.wiltonwebtoolkit;

public class HttpServerJni {

    static {
        System.loadLibrary("wilton_jni");
        try {
            Class.forName("net.wiltonwebtoolkit.HttpException");
            Class.forName("net.wiltonwebtoolkit.HttpGateway");
        } catch (ClassNotFoundException e) {
            throw new RuntimeException(e);
        }
    }

    public static native long createServer(Object gateway, String conf);

    public static native void stopServer(long serverHandle);

    public static native String getRequestMetadata(long requestHandle);

    public static native String getRequestData(long requestHandle);

    public static native void setResponseMetadata(long requestHandle, String conf);

    public static native void sendResponse(long requestHandle, String data);

//    TODO
//    private static native void sendResponseChunked(long requestHandle, Object readable);

    public static native void appendLog(int level, String logger, String message);
}
