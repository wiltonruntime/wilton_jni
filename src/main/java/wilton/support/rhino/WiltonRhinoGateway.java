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

package wilton.support.rhino;

import wilton.WiltonException;
import wilton.WiltonGateway;
import org.mozilla.javascript.Context;
import org.mozilla.javascript.Function;
import org.mozilla.javascript.Scriptable;

/**
 * User: alexkasko
 * Date: 2/11/17
 */
class WiltonRhinoGateway implements WiltonGateway {

    @Override
    public String runScript(String callbackScriptJson) throws Exception {
        Scriptable scope = WiltonRhinoEnvironment.threadScope();
        Context cx = Context.enter();
        try {
            Object funObj = scope.get("WILTON_run", scope);
            if (funObj instanceof Function) {
                Object args[] = {callbackScriptJson};
                Function fun = (Function) funObj;
                Object result = fun.call(cx, scope, scope, args);
                return Context.toString(result);
            } else {
                throw new WiltonException("Cannot access 'WILTON_run' function in global Rhino scope," +
                        " call data: [" + callbackScriptJson + "]");
            }
        } finally {
            Context.exit();
        }
    }
}
