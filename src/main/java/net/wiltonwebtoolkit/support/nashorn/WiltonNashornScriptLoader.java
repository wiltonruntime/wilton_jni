package net.wiltonwebtoolkit.support.nashorn;

import jdk.nashorn.api.scripting.AbstractJSObject;
import net.wiltonwebtoolkit.WiltonException;

import javax.script.ScriptContext;
import javax.script.ScriptEngine;
import java.io.File;
import java.util.Arrays;

import static net.wiltonwebtoolkit.support.common.Utils.readFileToString;

/**
 * User: alexkasko
 * Date: 2/11/17
 */
public class WiltonNashornScriptLoader extends AbstractJSObject {

    public WiltonNashornScriptLoader() {
    }

    @Override
    public boolean isFunction() {
        return true;
    }

    @Override
    public Object call(Object thiz, Object... args) {
        try {
            WiltonNashornEnvironment.checkInitialized();
            ScriptEngine engine = WiltonNashornEnvironment.engine();
            ScriptContext context = WiltonNashornEnvironment.scriptContext();
            for (Object arg : args) {
                String filePath = String.valueOf(arg);
                String sourceCode = readFileToString(new File(filePath));
                engine.eval(sourceCode, context);
            }
            return null;
        } catch (Exception e) {
            throw new WiltonException("Error loading script: " + Arrays.toString(args), e);
        }
    }

}
