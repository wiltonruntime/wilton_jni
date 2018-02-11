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

package net.wiltontoolkit.support.nashorn;

import jdk.nashorn.api.scripting.AbstractJSObject;
import net.wiltontoolkit.WiltonException;

import javax.script.ScriptContext;
import javax.script.ScriptEngine;
import java.io.File;
import java.util.Arrays;

import static net.wiltontoolkit.WiltonJni.wiltoncall;

/**
 * User: alexkasko
 * Date: 2/11/17
 */
public class WiltonNashornScriptLoader extends AbstractJSObject {

    private final ScriptEngine engine;
    private final ScriptContext context;

    public WiltonNashornScriptLoader(ScriptEngine engine, ScriptContext context) {
        this.engine = engine;
        this.context = context;
    }

    @Override
    public boolean isFunction() {
        return true;
    }

    @Override
    public Object call(Object thiz, Object... args) {
        try {
            for (Object arg : args) {
                String filePath = String.valueOf(arg);
                String sourceCode = wiltoncall("load_module_resource", filePath);
                StringBuilder wrapper = new StringBuilder();
                // todo: check why e.stack is undefined in WILTON_run
                wrapper.append("try {")
                        .append(sourceCode)
                        .append("\n} catch(e) {\n")
                        .append("throw new Error(e.stack);\n")
                        .append("}\n")
                        .append("//# sourceURL=")
                        .append(filePath);
//                wrapper.append(sourceCode)
//                        // see https://bugs.openjdk.java.net/browse/JDK-8032068
//                        .append("\n//# sourceURL=")
//                        .append(path);
                engine.eval(wrapper.toString(), context);
            }
            return null;
        } catch (Exception e) {
//            e.printStackTrace();
            throw new WiltonException("Error loading script: " + Arrays.toString(args) +
                    "\n" + e.getMessage(), e);
        }
    }

}
