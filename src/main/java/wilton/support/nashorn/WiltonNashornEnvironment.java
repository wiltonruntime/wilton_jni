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

package wilton.support.nashorn;

import wilton.WiltonException;
import wilton.WiltonGateway;

import javax.script.*;

/**
 * User: alexkasko
 * Date: 2/11/17
 */
public class WiltonNashornEnvironment {

    private static final WiltonGateway GATEWAY = new WiltonNashornGateway();
    private static ScriptEngine ENGINE;
    private static ThreadLocal<ScriptContext> THREAD_CONTEXT = new ThreadLocal<ScriptContext>();
    private static String THREAD_INIT_CODE = null;

    // must be called from main thread before any other
    // nashorn thread is started
    public static void initialize(String threadInitCode) {
        THREAD_INIT_CODE = threadInitCode;
        ENGINE = new ScriptEngineManager().getEngineByName("nashorn");
    }

    static ScriptContext threadScriptContext() {
        if (null == THREAD_CONTEXT.get()) {
            try {
                ScriptContext context = new SimpleScriptContext();
                Bindings bind = ENGINE.createBindings();
                bind.put("WILTON_load", new WiltonNashornScriptLoader(ENGINE, context));
                context.setBindings(bind, ScriptContext.ENGINE_SCOPE);
                if (null == THREAD_INIT_CODE) {
                    throw new WiltonException("Nashorn environment not initialized, use 'WiltonNashornEnvironment::initialize'");
                }
                ENGINE.eval(THREAD_INIT_CODE, context);
                THREAD_CONTEXT.set(context);
            } catch (Exception e) {
                throw new WiltonException("Nashorn environment thread initialization error," +
                        " thread: [" + Thread.currentThread().getName() + "]", e);
            }
        }
        return THREAD_CONTEXT.get();
    }

    public static WiltonGateway gateway() {
        return GATEWAY;
    }
}
