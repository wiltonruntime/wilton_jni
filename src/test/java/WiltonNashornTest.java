import org.junit.Test;

import javax.script.ScriptEngine;
import javax.script.ScriptEngineManager;
import java.io.*;

import static org.apache.commons.io.IOUtils.closeQuietly;

/**
 * User: alexkasko
 * Date: 5/15/16
 */
public class WiltonNashornTest {
    private static final File RUNTESTS_JS = new File("src/test/js/runtests.js");

    @Test
    public void test() throws Exception {
        ScriptEngine engine = new ScriptEngineManager().getEngineByName("nashorn");
        if (null == engine) {
            System.out.println("ERROR: Nashorn is not available, probably running on jdk7, skipping test");
            return;
        }
        InputStream is = null;
        try {
            is = new FileInputStream(RUNTESTS_JS);
            Reader re = new InputStreamReader(is, "UTF-8");
            engine.eval(re);
        } catch(Exception e) {
            throw new RuntimeException(e);
        } finally {
            closeQuietly(is);
        }
    }
}
