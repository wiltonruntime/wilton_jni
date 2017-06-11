import com.google.common.collect.ImmutableMap;
import net.wiltontoolkit.WiltonException;
import org.junit.BeforeClass;
import org.junit.Test;
import utils.TestGateway;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicReference;

import static net.wiltontoolkit.WiltonJni.LOGGING_DISABLE;
import static net.wiltontoolkit.WiltonJni.wiltoncall;
import static org.junit.Assert.assertEquals;
import static utils.TestUtils.GSON;
import static utils.TestUtils.initWiltonOnce;

/**
 * User: alexkasko
 * Date: 12/10/16
 */
public class SharedJniTest {

    @BeforeClass
    public static void init() {
        // init, no logging by default, enable it when needed
        initWiltonOnce(new TestGateway(), LOGGING_DISABLE);
    }

    @Test
    public void testPut() {
        assertEquals(null, wiltoncall("shared_get", GSON.toJson(ImmutableMap.builder()
                .put("key", "foo")
                .build())));
        assertEquals(null, wiltoncall("shared_put", GSON.toJson(ImmutableMap.builder()
                .put("key", "foo")
                .put("value", "bar")
                .build())));
        assertEquals("bar", wiltoncall("shared_get", GSON.toJson(ImmutableMap.builder()
                .put("key", "foo")
                .build())));
        assertEquals("bar", wiltoncall("shared_put", GSON.toJson(ImmutableMap.builder()
                .put("key", "foo")
                .put("value", "baz")
                .build())));
        assertEquals("baz", wiltoncall("shared_remove", GSON.toJson(ImmutableMap.builder()
                .put("key", "foo")
                .build())));
        assertEquals(null, wiltoncall("shared_get", GSON.toJson(ImmutableMap.builder()
                .put("key", "foo")
                .build())));
    }

    @Test
    public void testWait() throws Exception {
        wiltoncall("shared_put", GSON.toJson(ImmutableMap.builder()
                .put("key", "foo")
                .put("value", "bar")
                .build()));

        final CountDownLatch latch1 = new CountDownLatch(1);
        final CountDownLatch latch2 = new CountDownLatch(1);
        final AtomicReference<String> shared = new AtomicReference<String>("");
        new Thread(new Runnable() {
            @Override
            public void run() {
                latch1.countDown();
                String res = wiltoncall("shared_wait_change", GSON.toJson(ImmutableMap.builder()
                        .put("timeoutMillis", 10000)
                        .put("key", "foo")
                        .put("currentValue", "bar")
                        .build()));
                shared.set(res);
                latch2.countDown();
            }
        }).start();
        latch1.await();
        Thread.sleep(50);
        wiltoncall("shared_put", GSON.toJson(ImmutableMap.builder()
                .put("key", "foo")
                .put("value", "baz")
                .build()));
        latch2.await();
        assertEquals("baz", shared.get());

        wiltoncall("shared_remove", GSON.toJson(ImmutableMap.builder()
                .put("key", "foo")
                .build()));
    }

    @Test(expected = TestException.class)
    public void testWaitFail() {
        wiltoncall("shared_put", GSON.toJson(ImmutableMap.builder()
                .put("key", "foo")
                .put("value", "bar")
                .build()));
        try {
            wiltoncall("shared_wait_change", GSON.toJson(ImmutableMap.builder()
                    .put("timeoutMillis", 1000)
                    .put("key", "foo")
                    .put("currentValue", "bar")
                    .build()));
        } catch (WiltonException e) {
            throw new TestException(e);
        } finally {
            wiltoncall("shared_remove", GSON.toJson(ImmutableMap.builder()
                    .put("key", "foo")
                    .build()));
        }
    }

    private static final class TestException extends RuntimeException {
        private TestException(Throwable cause) {
            super(cause);
        }
    }

}
