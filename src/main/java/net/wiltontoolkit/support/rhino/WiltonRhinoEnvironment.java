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

package net.wiltontoolkit.support.rhino;

import net.wiltontoolkit.WiltonException;
import net.wiltontoolkit.WiltonGateway;
import net.wiltontoolkit.support.common.Utils;
import org.mozilla.javascript.*;

import java.io.*;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * User: alexkasko
 * Date: 2/11/17
 */
public class WiltonRhinoEnvironment {

    private static final WiltonGateway GATEWAY = new WiltonRhinoGateway();
    private static final AtomicBoolean INITIALIZED = new AtomicBoolean(false);
    private static final ThreadLocal<ScriptableObject> RHINO_THREAD_SCOPE = new ThreadLocal<ScriptableObject>();
    private static String INIT_CODE = "UNSPECIFIED";

    public static void initialize(String initCode) {
        if (!INITIALIZED.compareAndSet(false, true)) {
            throw new WiltonException("Rhino environment is already initialized");
        }
        try {
            INIT_CODE = initCode;
            ContextFactory.initGlobal(WiltonRhinoContextFactory.INSTANCE);
        } catch (Exception e) {
            throw new WiltonException("Rhino environment initialization error", e);
        }
    }

    public static Scriptable threadScope() {
        if (null == RHINO_THREAD_SCOPE.get()) {
            try {
                Context cx = Context.enter();
                ScriptableObject scope = cx.initStandardObjects();
                RHINO_THREAD_SCOPE.set(scope); // set early for loader
                FunctionObject loadFunc = new FunctionObject("load", WiltonRhinoScriptLoader.getLoadMethod(), scope);
                scope.put("WILTON_load", scope, loadFunc);
                scope.setAttributes("WILTON_load", ScriptableObject.DONTENUM);
                cx.evaluateString(scope, INIT_CODE, "WiltonRhinoEnvironment::initialize", -1, null);
                Context.exit();
            } catch (Exception e) {
                throw new WiltonException("Rhino environment thread initialization error," +
                        " thread: [" + Thread.currentThread().getName() + "]", e);
            }
        }
        return RHINO_THREAD_SCOPE.get();
    }

    public static WiltonGateway gateway() {
        return GATEWAY;
    }

    static void checkInitialized() {
        if (!INITIALIZED.get()) {
            throw new WiltonException("Rhino environment not initialized, use 'WiltonRhinoEnvironment::initialize'");
        }
    }

}
