package net.wiltontoolkit.support.rhino;

import net.wiltontoolkit.WiltonException;
import net.wiltontoolkit.WiltonGateway;
import net.wiltontoolkit.support.common.Utils;
import org.mozilla.javascript.*;

import java.io.*;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * User: alexkasko
 * Date: 2/11/17
 */
public class WiltonRhinoEnvironment {

    private static final WiltonGateway GATEWAY = new WiltonRhinoGateway();
    private static final AtomicBoolean INITIALIZED = new AtomicBoolean(false);
    private static ScriptableObject RHINO_GLOBAL_SCOPE = null;
    private static String INIT_THREAD = "";

    public static void initialize(String pathToScriptsDir) {
        if (!INITIALIZED.compareAndSet(false, true)) {
            throw new WiltonException("Rhino environment is already initialized, init thread: [" + INIT_THREAD + "]");
        }
        try {
            INIT_THREAD = Thread.currentThread().getName();
            ContextFactory.initGlobal(new WiltonRhinoContextFactory());
            Context cx = Context.enter();
            RHINO_GLOBAL_SCOPE = cx.initStandardObjects();
            FunctionObject loadFunc = new FunctionObject("load", WiltonRhinoScriptLoader.getLoadMethod(), RHINO_GLOBAL_SCOPE);
            RHINO_GLOBAL_SCOPE.put("WILTON_load", RHINO_GLOBAL_SCOPE, loadFunc);
            RHINO_GLOBAL_SCOPE.setAttributes("WILTON_load", ScriptableObject.DONTENUM);
            String reqjsPath = new File(pathToScriptsDir, "wilton-requirejs").getAbsolutePath() + File.separator;
            cx.evaluateString(RHINO_GLOBAL_SCOPE,
                    "WILTON_REQUIREJS_DIRECTORY = \"" + reqjsPath + "\"",
                    "WiltonRhinoEnvironment::initialize", -1, null);
            String modulesPath = new File(pathToScriptsDir, "wilton_modules").getAbsolutePath() + File.separator;
            cx.evaluateString(RHINO_GLOBAL_SCOPE,
                    "WILTON_REQUIREJS_CONFIG = '{" +
                        " \"waitSeconds\": 0," +
                        " \"enforceDefine\": true," +
                        " \"nodeIdCompat\": true," +
                        " \"baseUrl\": \"" + modulesPath + "\"" +
                    "}'",
                    "WiltonRhinoEnvironment::initialize", -1, null);
            // print() function
            cx.evaluateString(RHINO_GLOBAL_SCOPE,
                    "function print(msg) {" +
                     "    Packages.java.lang.System.out.println(msg);" +
                     "}", "WiltonRhinoEnvironment::initialize", -1, null);
            String code = Utils.readFileToString(new File(reqjsPath + "wilton-jni.js"));
            cx.evaluateString(RHINO_GLOBAL_SCOPE, code, "WiltonRhinoEnvironment::initialize", -1, null);
            Context.exit();
        } catch (Exception e) {
            throw new WiltonException("Rhino environment initialization error", e);
        }
    }

    public static Scriptable globalScope() {
        return RHINO_GLOBAL_SCOPE;
    }

    public static WiltonGateway gateway() {
        return GATEWAY;
    }

    static void checkInitialized() {
        if (!INITIALIZED.get()) {
            throw new WiltonException("Rhino environment not initialized, use 'WiltonRhinoEnvironment::initialize'");
        }
    }

}
