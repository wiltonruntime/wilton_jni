import org.junit.Test;
import org.mozilla.javascript.Context;
import org.mozilla.javascript.tools.shell.Global;

import java.io.File;

import static utils.TestUtils.runRhinoFs;
import static utils.TestUtils.runRhinoClasspath;

/**
 * User: alexkasko
 * Date: 5/15/16
 */
public class WiltonRhinoTest {

    @Test
    public void test() throws Exception {
        Context cx = Context.enter();
        cx.setOptimizationLevel(-1);
        Global gl = new Global();
        gl.init(cx);
        runRhinoFs(cx, gl, new File("js/wilton.js"));
        runRhinoClasspath(cx, gl, "/wilton_test.js");
        Context.exit();
    }

    @Test
    public void testRequire() throws Exception {
        Context cx = Context.enter();
        cx.setOptimizationLevel(-1);
        Global gl = new Global();
        gl.init(cx);
        runRhinoFs(cx, gl, new File("js/require.js"));
        runRhinoFs(cx, gl, new File("js/rhino.js"));
        runRhinoClasspath(cx, gl, "/wilton-require_test.js");
        Context.exit();
    }
}
