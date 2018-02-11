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

import org.mozilla.javascript.Context;
import org.mozilla.javascript.Function;
import org.mozilla.javascript.Script;
import org.mozilla.javascript.Scriptable;

import java.lang.reflect.Method;

import static net.wiltontoolkit.WiltonJni.wiltoncall;

/**
 * User: alexkasko
 * Date: 2/11/17
 */
class WiltonRhinoScriptLoader {

    public static void load(Context cx, Scriptable thisObj, Object[] args, Function funObj) throws Exception {
        WiltonRhinoEnvironment.checkInitialized();
        for (Object arg : args) {
            String filePath = Context.toString(arg);
            String sourceCode = wiltoncall("load_module_resource", filePath);
            Script script = cx.compileString(sourceCode, filePath, 1, null);
            if (script != null) {
                script.exec(cx, WiltonRhinoEnvironment.threadScope());
            }
        }
    }

    static Method getLoadMethod() {
        try {
            return WiltonRhinoScriptLoader.class.getMethod("load", Context.class,
                    Scriptable.class, Object[].class, Function.class);
        } catch (NoSuchMethodException e) {
            throw new RuntimeException(e);
        }
    }

}
