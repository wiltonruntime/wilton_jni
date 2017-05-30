package net.wiltontoolkit.support.nashorn;

import jdk.nashorn.api.scripting.JSObject;
import net.wiltontoolkit.WiltonException;
import net.wiltontoolkit.WiltonGateway;

import javax.script.*;

/**
 * User: alexkasko
 * Date: 2/11/17
 */
class WiltonNashornGateway implements WiltonGateway {

    @Override
    public String runScript(String callbackScriptJson) throws Exception {
        WiltonNashornEnvironment.checkInitialized();
        ScriptContext context = WiltonNashornEnvironment.scriptContext();
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
