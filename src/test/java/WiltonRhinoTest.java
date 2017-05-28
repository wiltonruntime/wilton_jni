import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableMap;
import net.wiltonwebtoolkit.support.rhino.WiltonRhinoEnvironment;
import org.junit.BeforeClass;
import org.junit.Test;
import org.mozilla.javascript.*;
import utils.TestGateway;
import utils.TestUtils;

import java.io.*;

import static net.wiltonwebtoolkit.WiltonJni.LOGGING_DISABLE;
import static org.apache.commons.io.FileUtils.readFileToString;
import static org.apache.commons.io.IOUtils.closeQuietly;
import static org.apache.commons.io.IOUtils.toString;
import static org.junit.Assert.assertEquals;
import static utils.TestUtils.GSON;
import static utils.TestUtils.getJsDir;
import static utils.TestUtils.initWiltonOnce;

/**
 * User: alexkasko
 * Date: 5/15/16
 */
public class WiltonRhinoTest {

    @BeforeClass
    public static void init() {
        // init, no logging by default, enable it when needed
        initWiltonOnce(new TestGateway(), LOGGING_DISABLE);
        WiltonRhinoEnvironment.initialize(getJsDir().getAbsolutePath());
        TestGateway tg = (TestGateway) TestUtils.GATEWAY;
        tg.setScriptGateway(WiltonRhinoEnvironment.gateway());
    }

    @Test
    public void test() throws Exception {
        // wilton test suite
        WiltonRhinoEnvironment.gateway().runScript(GSON.toJson(ImmutableMap.builder()
                .put("module", "tests/runtests")
                .put("func", "runTests")
                .put("args", ImmutableList.of())
                .build()));
        // node modules tests
        WiltonRhinoEnvironment.gateway().runScript(GSON.toJson(ImmutableMap.builder()
                .put("module", "tests/runNodeTests")
                .put("func", "")
                .put("args", ImmutableList.of())
                .build()));
    }

    @Test
    // https://github.com/mozilla/rhino/issues/153
    public void testStacktrace() throws Exception {
        Scriptable scope = WiltonRhinoEnvironment.globalScope();
        Context cx = Context.enter();
        Object[] holder = new Object[1];
        ScriptableObject.putProperty(scope, "holder", Context.javaToJS(holder, scope));
        cx.evaluateString(scope, "try {throw new Error() } catch (e) {holder[0] = e.stack}", "some_file.js", 42, null);
        assertEquals("\tat some_file.js:42\n", holder[0]);
        Context.exit();
    }
}
