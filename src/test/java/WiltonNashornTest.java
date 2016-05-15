import org.junit.Test;

import javax.script.ScriptEngine;
import javax.script.ScriptEngineManager;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;

import static org.apache.commons.io.IOUtils.closeQuietly;

/**
 * User: alexkasko
 * Date: 5/15/16
 */
public class WiltonNashornTest {
    @Test
    public void test() throws Exception {
        ScriptEngine engine = new ScriptEngineManager().getEngineByName("nashorn");
        if (null == engine) {
            System.out.println("ERROR: Nashorn is not available, probably running in jdk7, skipping test");
            return;
        }
        {
            InputStream is = null;
            try {
                is = new FileInputStream("js/wilton.js");
                Reader re = new InputStreamReader(is, "UTF-8");
                engine.eval(re);
            } finally {
                closeQuietly(is);
            }
        }
        {
            InputStream is = null;
            try {
                is = WiltonRhinoTest.class.getResourceAsStream("/wilton_test.js");
                Reader re = new InputStreamReader(is, "UTF-8");
                engine.eval(re);
            } finally {
                closeQuietly(is);
            }
        }
    }
}
