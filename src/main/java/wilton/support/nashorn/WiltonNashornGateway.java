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

import jdk.nashorn.api.scripting.JSObject;
import wilton.WiltonException;
import wilton.WiltonGateway;

import javax.script.*;

/**
 * User: alexkasko
 * Date: 2/11/17
 */
class WiltonNashornGateway implements WiltonGateway {

    @Override
    public String runScript(String callbackScriptJson) throws Exception {
        ScriptContext context = WiltonNashornEnvironment.threadScriptContext();
        JSObject jsObject = (JSObject) context.getAttribute("WILTON_run", ScriptContext.ENGINE_SCOPE);
        if (null != jsObject) {
            Object result = jsObject.call(null, callbackScriptJson);
            return result.toString();
        } else {
            throw new WiltonException("Cannot access 'WILTON_run' function in thread-local Nashorn scope," +
                    " call data: [" + callbackScriptJson + "], current thread: [" + Thread.currentThread().getName() + "]");
        }
    }
}
