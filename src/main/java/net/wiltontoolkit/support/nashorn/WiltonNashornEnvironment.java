package net.wiltontoolkit.support.nashorn;

import net.wiltontoolkit.WiltonException;
import net.wiltontoolkit.WiltonGateway;
import net.wiltontoolkit.support.common.Utils;

import javax.script.*;
import java.io.File;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * User: alexkasko
 * Date: 2/11/17
 */
public class WiltonNashornEnvironment {

    private static final WiltonGateway GATEWAY = new WiltonNashornGateway();
    private static final AtomicBoolean INITIALIZED = new AtomicBoolean(false);
    private static ScriptEngine ENGINE;
    private static String SCRIPTS_DIR_PATH = "SPECIFY_ME";
    private static String INIT_THREAD = "";
    private static ThreadLocal<ScriptContext> THREAD_CONTEXT = new ThreadLocal<ScriptContext>();

    public static void initialize(String pathToScriptsDir) {
        if (!INITIALIZED.compareAndSet(false, true)) {
            throw new WiltonException("Nashorn environment is already initialized, init thread: [" + INIT_THREAD + "]");
        }
        try {
            INIT_THREAD = Thread.currentThread().getName();
            SCRIPTS_DIR_PATH = pathToScriptsDir;
            ENGINE = new ScriptEngineManager().getEngineByName("nashorn");
        } catch (Exception e) {
            throw new WiltonException("Nashorn environment initialization error", e);
        }
    }

    public static ScriptEngine engine() {
        return ENGINE;
    }

    public static WiltonGateway gateway() {
        return GATEWAY;
    }

    static ScriptContext scriptContext() {
        if (null == THREAD_CONTEXT.get()) {
            ScriptContext context = new SimpleScriptContext();
            Bindings bind = ENGINE.createBindings();
            bind.put("WILTON_load", new WiltonNashornScriptLoader(ENGINE, context));
            context.setBindings(bind, ScriptContext.ENGINE_SCOPE);
            String reqjsPath = new File(SCRIPTS_DIR_PATH, "wilton-requirejs").getAbsolutePath() + File.separator;
            String modulesPath = new File(SCRIPTS_DIR_PATH, "modules").getAbsolutePath() + File.separator;
            try {
                ENGINE.eval("WILTON_REQUIREJS_DIRECTORY = \"" + reqjsPath + "\"", context);
                ENGINE.eval("WILTON_REQUIREJS_CONFIG = '{" +
                        " \"waitSeconds\": 0," +
                        " \"enforceDefine\": true," +
                        " \"nodeIdCompat\": true," +
                        " \"baseUrl\": \"" + modulesPath + "\"" +
                        "}'", context);
                String code = Utils.readFileToString(new File(reqjsPath + "wilton-jni.js"));
                ENGINE.eval(code, context);
            } catch (Exception e) {
                throw new WiltonException("Nashorn environment thread initialization error," +
                        " thread: [" + Thread.currentThread().getName() + "]", e);
            }
            THREAD_CONTEXT.set(context);
        }
        return THREAD_CONTEXT.get();
    }

    static void checkInitialized() {
        if (!INITIALIZED.get()) {
            throw new WiltonException("Nashorn environment not initialized, use 'WiltonNashornEnvironment::initialize'");
        }
    }
}
