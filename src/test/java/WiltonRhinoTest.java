import net.wiltonwebtoolkit.support.rhino.WiltonContextFactory;
import org.junit.Test;
import org.mozilla.javascript.Context;
import org.mozilla.javascript.ScriptableObject;
import org.mozilla.javascript.tools.shell.Global;

import java.io.*;

import static org.apache.commons.io.IOUtils.closeQuietly;
import static org.junit.Assert.assertEquals;

/**
 * User: alexkasko
 * Date: 5/15/16
 */
public class WiltonRhinoTest {
    private static final File RUNTESTS_JS = new File("src/test/js/runtests.js");


    @Test
    public void test() throws Exception {
        Context cx = new WiltonContextFactory().enterContext();
        Global gl = new Global();
        gl.init(cx);

        InputStream is = null;
        try {
            is = new FileInputStream(RUNTESTS_JS.getAbsolutePath());
            Reader re = new InputStreamReader(is, "UTF-8");
            cx.evaluateReader(gl, re, RUNTESTS_JS.getName(), -1, null);
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
