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

public class WiltonJni {

    static {
        System.loadLibrary("wilton_jni");
        try {
            Class.forName("net.wiltonwebtoolkit.WiltonException");
            Class.forName("net.wiltonwebtoolkit.WiltonGateway");
        } catch (ClassNotFoundException e) {
            throw new RuntimeException(e);
        }
    }

    // unified jni access point

    public static String wiltoncall(String name) {
        return wiltoncall(name, "{}", null);
    }

    public static String wiltoncall(String name, String data) {
        return wiltoncall(name, data, null);
    }

    public static native String wiltoncall(String name, String data, Object object);

    // server

    public static native long createServer(Object gateway, String conf) throws WiltonException;

    public static native void stopServer(long serverHandle) throws WiltonException;

    public static native String getRequestMetadata(long requestHandle) throws WiltonException;

    public static native String getRequestData(long requestHandle) throws WiltonException;

    public static native String getRequestDataFilename(long requestHandle) throws WiltonException;

    public static native void setResponseMetadata(long requestHandle, String conf) throws WiltonException;

    public static native void sendResponse(long requestHandle, String data) throws WiltonException;

    public static native void sendTempFile(long requestHandle, String filePath) throws WiltonException;

    public static native void sendMustache(long requestHandle, String mustacheFilePath, String valuesJson) throws WiltonException;

    public static native long sendLater(long requestHandle) throws WiltonException;

    public static native long sendWithResponseWriter(long responseWriterHandle, String data) throws WiltonException;

    // log

    public static native void appendLog(String level, String logger, String message) throws WiltonException;

    // DB

    public static native long openDbConnection(String url) throws WiltonException;

    public static native String dbQuery(long connectionHandle, String sql, String paramsJson) throws WiltonException;

    public static native void dbExecute(long connectionHandle, String sql, String paramsJson) throws WiltonException;

    public static native void closeDbConnection(long connectionHandle) throws WiltonException;

    public static native long startDbTransaction(long connectionHandle) throws WiltonException;

    public static native void commitDbTransaction(long transactionHandle) throws WiltonException;

    public static native void rollbackDbTransaction(long transactionHandle) throws WiltonException;

}
