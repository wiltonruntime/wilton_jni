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
    private static ThreadLocal<ScriptableObject> RHINO_THREAD_SCOPE = new ThreadLocal<ScriptableObject>();
    private static String INIT_THREAD = "";
    private static String PATH_TO_SCRIPTS_DIR;

    public static void initialize(String pathToScriptsDir) {
        if (!INITIALIZED.compareAndSet(false, true)) {
            throw new WiltonException("Rhino environment is already initialized, init thread: [" + INIT_THREAD + "]");
        }
        try {
            INIT_THREAD = Thread.currentThread().getName();
            PATH_TO_SCRIPTS_DIR = pathToScriptsDir;
            ContextFactory.initGlobal(new WiltonRhinoContextFactory());
        } catch (Exception e) {
            throw new WiltonException("Rhino environment initialization error", e);
        }
    }

    public static Scriptable threadScope() {
        if (null == RHINO_THREAD_SCOPE.get()) {
            try {
                Context cx = Context.enter();
                ScriptableObject scope = cx.initStandardObjects();
                RHINO_THREAD_SCOPE.set(scope); // set early for loader
                FunctionObject loadFunc = new FunctionObject("load", WiltonRhinoScriptLoader.getLoadMethod(), scope);
                scope.put("WILTON_load", scope, loadFunc);
                scope.setAttributes("WILTON_load", ScriptableObject.DONTENUM);
                String reqjsPath = new File(PATH_TO_SCRIPTS_DIR, "wilton-requirejs").getAbsolutePath() + File.separator;
                cx.evaluateString(scope,
                        "WILTON_REQUIREJS_DIRECTORY = \"" + reqjsPath + "\"",
                        "WiltonRhinoEnvironment::initialize", -1, null);
                String modulesPath = new File(PATH_TO_SCRIPTS_DIR, "modules").getAbsolutePath() + File.separator;
                cx.evaluateString(scope,
                        "WILTON_REQUIREJS_CONFIG = '{" +
                                " \"waitSeconds\": 0," +
                                " \"enforceDefine\": true," +
                                " \"nodeIdCompat\": true," +
                                " \"baseUrl\": \"" + modulesPath + "\"" +
                                "}'",
                        "WiltonRhinoEnvironment::initialize", -1, null);
                // print() function
                cx.evaluateString(scope,
                        "function print(msg) {" +
                                "    Packages.java.lang.System.out.println(msg);" +
                                "}", "WiltonRhinoEnvironment::initialize", -1, null);
                String code = Utils.readFileToString(new File(reqjsPath + "wilton-jni.js"));
                cx.evaluateString(scope, code, "WiltonRhinoEnvironment::initialize", -1, null);
                Context.exit();
            } catch (Exception e) {
                throw new WiltonException("Rhino environment thread initialization error," +
                        " thread: [" + Thread.currentThread().getName() + "]", e);
            }
        }
        return RHINO_THREAD_SCOPE.get();
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
