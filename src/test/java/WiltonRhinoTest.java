import net.wiltonwebtoolkit.support.rhino.WiltonContextFactory;
import org.apache.commons.io.Charsets;
import org.apache.commons.io.FileUtils;
import org.junit.BeforeClass;
import org.junit.Test;
import org.mozilla.javascript.*;
import utils.TestGateway;

import java.io.*;
import java.lang.reflect.Method;

import static net.wiltonwebtoolkit.WiltonJni.LOGGING_DISABLE;
import static org.apache.commons.io.FileUtils.readFileToString;
import static org.apache.commons.io.IOUtils.closeQuietly;
import static org.junit.Assert.assertEquals;
import static utils.TestUtils.initWiltonOnce;

/**
 * User: alexkasko
 * Date: 5/15/16
 */
public class WiltonRhinoTest {
    private static final File RUNTESTS_JS = new File("src/test/js/runtests.js");

    public static final ScriptableObject RHINO_GLOBAL_SCOPE;

    // todo: to non-test support utils

    static {
        ContextFactory.initGlobal(new WiltonContextFactory());
        Context cx = Context.enter();
        RHINO_GLOBAL_SCOPE = cx.initStandardObjects();
        FunctionObject loadFunc = new FunctionObject("load", RhinoLoader.getLoadMethod(), RHINO_GLOBAL_SCOPE);
        RHINO_GLOBAL_SCOPE.put("load", RHINO_GLOBAL_SCOPE, loadFunc);
        RHINO_GLOBAL_SCOPE.setAttributes("load", ScriptableObject.DONTENUM);
        Context.exit();
    }

    private static class RhinoLoader {

        public static void load(Context cx, Scriptable thisObj, Object[] args, Function funObj) throws Exception {
            for (Object arg : args) {
                String file = Context.toString(arg);
                String sourceCode = readFileToString(new File(".", file), Charsets.UTF_8);
                Script script = cx.compileString(sourceCode, file, 1, null);
                if (script != null) {
                    script.exec(cx, RHINO_GLOBAL_SCOPE);
                }
            }
        }

        public static Method getLoadMethod() {
            try {
                return RhinoLoader.class.getMethod("load", Context.class, Scriptable.class, Object[].class, Function.class);
            } catch (NoSuchMethodException e) {
                throw new RuntimeException(e);
            }
        }
    }

    @BeforeClass
    public static void init() {
        // init, no logging by default, enable it when needed
        initWiltonOnce(new TestGateway(), LOGGING_DISABLE);
    }

    @Test
    public void test() throws Exception {
        Context cx = Context.enter();
        InputStream is = null;
        try {
            is = new FileInputStream(RUNTESTS_JS.getAbsolutePath());
            Reader re = new InputStreamReader(is, "UTF-8");
            cx.evaluateReader(RHINO_GLOBAL_SCOPE, re, RUNTESTS_JS.getName(), -1, null);
        } catch(Exception e) {
            throw new RuntimeException(e);
        } finally {
            closeQuietly(is);
        }
        Context.exit();
    }

    @Test
    // https://github.com/mozilla/rhino/issues/153
    public void testStacktrace() throws Exception {
        Context cx = Context.enter();
        Object[] holder = new Object[1];
        ScriptableObject.putProperty(RHINO_GLOBAL_SCOPE, "holder", Context.javaToJS(holder, RHINO_GLOBAL_SCOPE));
        cx.evaluateString(RHINO_GLOBAL_SCOPE, "try {throw new Error() } catch (e) {holder[0] = e.stack}", "some_file.js", 42, null);
        assertEquals("\tat some_file.js:42\n", holder[0]);
        Context.exit();
    }
}
