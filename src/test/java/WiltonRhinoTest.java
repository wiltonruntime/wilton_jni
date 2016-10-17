import net.wiltonwebtoolkit.support.rhino.WiltonContextFactory;
import org.junit.Test;
import org.mozilla.javascript.Context;
import org.mozilla.javascript.ScriptableObject;
import org.mozilla.javascript.tools.shell.Global;

import java.io.File;

import static org.junit.Assert.assertEquals;
import static utils.TestUtils.runRhinoFs;
import static utils.TestUtils.runRhinoClasspath;

/**
 * User: alexkasko
 * Date: 5/15/16
 */
public class WiltonRhinoTest {

    @Test
    public void test() throws Exception {
        Context cx = new WiltonContextFactory().enterContext();
        Global gl = new Global();
        gl.init(cx);
        runRhinoFs(cx, gl, new File("js/wilton.js"));
        runRhinoClasspath(cx, gl, "/wilton_test.js");
        Context.exit();
    }

    @Test
    public void testRequire() throws Exception {
        Context cx = new WiltonContextFactory().enterContext();
        Global gl = new Global();
        gl.init(cx);
        runRhinoFs(cx, gl, new File("js/require.js"));
        runRhinoFs(cx, gl, new File("js/rhino.js"));
        runRhinoClasspath(cx, gl, "/wilton-require_test.js");
        Context.exit();
    }

    @Test
    // https://github.com/mozilla/rhino/issues/153
    public void testStacktrace() throws Exception {
        Context cx = new WiltonContextFactory().enterContext();
        Global gl = new Global();
        gl.init(cx);
        Object[] holder = new Object[1];
        ScriptableObject.putProperty(gl, "holder", Context.javaToJS(holder, gl));
        cx.evaluateString(gl, "try {throw new Error() } catch (e) {holder[0] = e.stack}", "some_file.js", 42, null);
        assertEquals("\tat some_file.js:42\n", holder[0]);
        Context.exit();
    }
}
