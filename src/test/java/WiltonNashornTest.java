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
        // init, no logging by default, enable it when needed
        initWiltonOnce(new TestGateway(), LOGGING_DISABLE);
        TestGateway tg = (TestGateway) TestUtils.GATEWAY;
        WiltonNashornEnvironment.initialize(getJsDir().getAbsolutePath());
        tg.setScriptGateway(WiltonNashornEnvironment.gateway());
    }

    @Test
    public void test() throws Exception {
        // wilton test suite
        WiltonNashornEnvironment.gateway().runScript(GSON.toJson(ImmutableMap.builder()
                .put("module", "runWiltonTests")
                .put("func", "main")
                .put("args", ImmutableList.of())
                .build()));
        // node modules tests
        WiltonNashornEnvironment.gateway().runScript(GSON.toJson(ImmutableMap.builder()
                .put("module", "runNodeTests")
                .put("func", "")
                .put("args", ImmutableList.of())
                .build()));
    }
}
