package net.wiltonwebtoolkit.support.rhino;

import net.wiltonwebtoolkit.WiltonException;
import net.wiltonwebtoolkit.WiltonGateway;
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
    private static String PATH_TO_SCRIPTS_DIR = "SPECIFY_ME";
    private static String INIT_THREAD = "";

    public static void initialize(String pathToScriptsDir) {
        if (!INITIALIZED.compareAndSet(false, true)) {
            throw new WiltonException("Rhino environment is already initialized, init thread: [" + INIT_THREAD + "]");
        }
        try {
            INIT_THREAD = Thread.currentThread().getName();
            PATH_TO_SCRIPTS_DIR = pathToScriptsDir;
            ContextFactory.initGlobal(new WiltonRhinoContextFactory());
            Context cx = Context.enter();
            RHINO_GLOBAL_SCOPE = cx.initStandardObjects();
            FunctionObject loadFunc = new FunctionObject("load", WiltonRhinoScriptLoader.getLoadMethod(), RHINO_GLOBAL_SCOPE);
            RHINO_GLOBAL_SCOPE.put("load", RHINO_GLOBAL_SCOPE, loadFunc);
            RHINO_GLOBAL_SCOPE.setAttributes("load", ScriptableObject.DONTENUM);
            String reqjsPath = new File(PATH_TO_SCRIPTS_DIR, "requirejs").getAbsolutePath() + File.separator;
            String modulesPath = new File(PATH_TO_SCRIPTS_DIR, "modules").getAbsolutePath() + File.separator;
            cx.evaluateString(RHINO_GLOBAL_SCOPE,
                    "(function() {" +
                    "   load('" + reqjsPath + "require.js');" +
                    "   load('" + reqjsPath + "loader.js');" +
                    "   load('" + reqjsPath + "runner.js');" +
                    "   requirejs.config({" +
                    "       baseUrl: '" + modulesPath + "'" +
                    "   });" +
                    "}());", "WiltonRhinoEnvironment::initialize", -1, null);
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
