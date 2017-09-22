import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableMap;
import net.wiltontoolkit.support.nashorn.WiltonNashornEnvironment;
import org.junit.BeforeClass;
import org.junit.Test;
import utils.TestGateway;
import utils.TestUtils;

import static net.wiltontoolkit.WiltonJni.LOGGING_DISABLE;
import static utils.TestUtils.GSON;
import static utils.TestUtils.getJsDir;
import static utils.TestUtils.initWiltonOnce;

/**
 * User: alexkasko
 * Date: 5/15/16
 */
public class WiltonNashornTest {

    @BeforeClass
    public static void init() {
        String wiltonDirPath = getJsDir().getAbsolutePath();
        // init, no logging by default, enable it when needed
        initWiltonOnce(new TestGateway(), LOGGING_DISABLE, wiltonDirPath);
        TestGateway tg = (TestGateway) TestUtils.GATEWAY;
        WiltonNashornEnvironment.initialize(wiltonDirPath);
        tg.setScriptGateway(WiltonNashornEnvironment.gateway());
    }

    @Test
    public void test() throws Exception {
        // wilton test suite
        WiltonNashornEnvironment.gateway().runScript(GSON.toJson(ImmutableMap.builder()
                .put("module", "../js/wilton/test")
                .put("func", "main")
                .build()));
        // node modules tests
        WiltonNashornEnvironment.gateway().runScript(GSON.toJson(ImmutableMap.builder()
                .put("module", "test/scripts/runNodeTests")
                .put("func", "")
                .build()));
    }
}
