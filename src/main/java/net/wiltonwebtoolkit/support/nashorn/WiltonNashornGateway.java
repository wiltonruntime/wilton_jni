package net.wiltonwebtoolkit.support.nashorn;

import jdk.nashorn.api.scripting.JSObject;
import net.wiltonwebtoolkit.WiltonException;
import net.wiltonwebtoolkit.WiltonGateway;

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
        JSObject jsObject = (JSObject) context.getAttribute("runScript", ScriptContext.ENGINE_SCOPE);
        if (null != jsObject) {
            Object result = jsObject.call(null, callbackScriptJson);
            return result.toString();
        } else {
            throw new WiltonException("Cannot access 'runScript' function in thread-local Nashorn scope," +
                    " call data: [" + callbackScriptJson + "], current thread: [" + Thread.currentThread().getName() + "]");
        }
    }
}
