import com.google.common.collect.ImmutableMap;
import org.junit.Test;

import static net.wiltonwebtoolkit.WiltonJni.wiltoncall;
import static org.junit.Assert.assertTrue;
import static utils.TestUtils.GSON;

/**
 * User: alexkasko
 * Date: 10/4/16
 */
public class CommonJniTest {

    @Test
    public void test() throws Exception {
        long start = System.currentTimeMillis();
        wiltoncall("sleep_millis", GSON.toJson(ImmutableMap.builder()
                .put("millis", 100)
                .build()));
        long stop = System.currentTimeMillis();
        assertTrue((stop - start) > 70);
    }
}
