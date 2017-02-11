package net.wiltonwebtoolkit.support.rhino;

import org.mozilla.javascript.Context;
import org.mozilla.javascript.Function;
import org.mozilla.javascript.Script;
import org.mozilla.javascript.Scriptable;

import java.io.*;
import java.lang.reflect.Method;

import static net.wiltonwebtoolkit.support.common.Utils.readFileToString;

/**
 * User: alexkasko
 * Date: 2/11/17
 */
class WiltonRhinoScriptLoader {

    public static void load(Context cx, Scriptable thisObj, Object[] args, Function funObj) throws Exception {
        WiltonRhinoEnvironment.checkInitialized();
        for (Object arg : args) {
            String filePath = Context.toString(arg);
            String sourceCode = readFileToString(new File(filePath));
            Script script = cx.compileString(sourceCode, filePath, 1, null);
            if (script != null) {
                script.exec(cx, WiltonRhinoEnvironment.globalScope());
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
