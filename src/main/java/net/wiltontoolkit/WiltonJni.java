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
package net.wiltontoolkit;

import java.io.PrintWriter;
import java.io.StringWriter;

public class WiltonJni {

    public static final String LOGGING_DISABLE = "{\"appenders\":[{\"appenderType\":\"NULL\"}]}";
    public static final String LOGGING_CONSOLE = "{\"appenders\":[{\"appenderType\":\"CONSOLE\"}]}";

    static {
        System.loadLibrary("wilton");
    }

    public static String describeThrowable(Throwable throwable) {
        if (null == throwable) {
            return "";
        }
        StringWriter sw = new StringWriter();
        PrintWriter pw = new PrintWriter(sw, true);
        throwable.printStackTrace(pw);
        return sw.getBuffer().toString();
    }

    // jni access points

//    public static void wiltoninit(WiltonGateway gateway) throws WiltonException {
//        wiltoninit(gateway, LOGGING_DISABLE);
//    }

    public static native void wiltoninit(WiltonGateway gateway, String config) throws WiltonException;

    public static String wiltoncall(String name) throws WiltonException {
        return wiltoncall(name, "{}");
    }

    public static native String wiltoncall(String name, String data) throws WiltonException;

}
