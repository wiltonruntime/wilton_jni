import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableMap;
import org.junit.BeforeClass;
import org.junit.Test;
import utils.TestGateway;

import static net.wiltontoolkit.WiltonJni.LOGGING_DISABLE;
import static net.wiltontoolkit.WiltonJni.wiltoncall;
import static org.junit.Assert.assertTrue;
import static utils.TestUtils.GSON;
import static utils.TestUtils.initWiltonOnce;

/**
 * User: alexkasko
 * Date: 12/10/16
 */
public class ThreadJniTest {

    @BeforeClass
    public static void init() {
        // init, no logging by default, enable it when needed
        initWiltonOnce(new TestGateway(), LOGGING_DISABLE);
    }

    @Test
    public void testRun() throws Exception {
        wiltoncall("thread_run", GSON.toJson(ImmutableMap.builder()
                .put("callbackScript", ImmutableMap.builder()
                        .put("module", "thread/test")
                        .put("func", "testRun")
                        .put("args", ImmutableList.of())
                        .build())
                .build()));
        TestGateway.threadTestLatch.await();
        assertTrue(Thread.currentThread().getId() != TestGateway.threadTestId.get());
    }

    @Test
    public void testSleep() throws Exception {
        long start = System.currentTimeMillis();
        wiltoncall("thread_sleep_millis", GSON.toJson(ImmutableMap.builder()
                .put("millis", 50)
                .build()));
        long stop = System.currentTimeMillis();
        assertTrue((stop - start) > 30);
    }
}
