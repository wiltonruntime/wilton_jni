import org.junit.Test;

import javax.script.ScriptEngine;
import javax.script.ScriptEngineManager;
import java.io.*;

import static utils.TestUtils.*;

/**
 * User: alexkasko
 * Date: 5/15/16
 */
public class WiltonNashornTest {
    @Test
    public void test() throws Exception {
        ScriptEngine engine = new ScriptEngineManager().getEngineByName("nashorn");
        if (null == engine) {
            System.out.println("ERROR: Nashorn is not available, probably running on jdk7, skipping test");
            return;
        }
        runNashornFs(engine, new File("js/wilton.js"));
        runNashornClasspath(engine, "/wilton_test.js");
    }

    @Test
    public void testRequire() throws Exception {
        ScriptEngine engine = new ScriptEngineManager().getEngineByName("nashorn");
        if (null == engine) {
            System.out.println("ERROR: Nashorn is not available, probably running on jdk7, skipping test");
            return;
        }
        engine.eval("requirejs = {config: {baseUrl: 'js/'}}");
        runNashornFs(engine, new File("js/wilton-require.js"));
        runNashornClasspath(engine, "/wilton-require_test.js");
    }
}
