package net.wiltontoolkit.support.rhino;

import net.wiltontoolkit.WiltonException;
import net.wiltontoolkit.WiltonGateway;
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
        WiltonRhinoEnvironment.checkInitialized();
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
