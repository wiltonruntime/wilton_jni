import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableMap;
import net.wiltonwebtoolkit.WiltonException;
import org.junit.BeforeClass;
import org.junit.Test;
import utils.TestGateway;

import java.util.Map;
import java.util.concurrent.atomic.AtomicBoolean;

import static net.wiltonwebtoolkit.WiltonJni.LOGGING_DISABLE;
import static net.wiltonwebtoolkit.WiltonJni.wiltoncall;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static utils.TestUtils.GSON;
import static utils.TestUtils.LONG_MAP_TYPE;
import static utils.TestUtils.initWiltonOnce;

/**
 * User: alexkasko
 * Date: 10/4/16
 */
public class MutexJniTest {

    @BeforeClass
    public static void init() {
        // init, no logging by default, enable it when needed
        initWiltonOnce(new TestGateway(), LOGGING_DISABLE);
    }

    @Test
    public void testLock() throws Exception {
        String out = wiltoncall("mutex_create");
        Map<String, Long> hamap = GSON.fromJson(out, LONG_MAP_TYPE);
        final long mutexHandle = hamap.get("mutexHandle");
        wiltoncall("mutex_lock", GSON.toJson(ImmutableMap.builder()
                .put("mutexHandle", mutexHandle)
                .build()));
        final AtomicBoolean shared = new AtomicBoolean(false);
        new Thread(new Runnable() {
            @Override
            public void run() {
                wiltoncall("mutex_lock", GSON.toJson(ImmutableMap.builder()
                        .put("mutexHandle", mutexHandle)
                        .build()));
                shared.set(true);
                wiltoncall("mutex_unlock", GSON.toJson(ImmutableMap.builder()
                        .put("mutexHandle", mutexHandle)
                        .build()));
            }
        }).start();
        assertFalse(shared.get());
        wiltoncall("mutex_unlock", GSON.toJson(ImmutableMap.builder()
                .put("mutexHandle", mutexHandle)
                .build()));
        Thread.sleep(100);
        assertTrue(shared.get());
        wiltoncall("mutex_destroy", GSON.toJson(ImmutableMap.builder()
                .put("mutexHandle", mutexHandle)
                .build()));
    }

    @Test
    public void testWaitNotify() throws Exception {
        String out = wiltoncall("mutex_create");
        Map<String, Long> hamap = GSON.fromJson(out, LONG_MAP_TYPE);
        final long mutexHandle = hamap.get("mutexHandle");
        Runnable waiter = new Runnable() {
            @Override
            public void run() {
                wiltoncall("mutex_lock", GSON.toJson(ImmutableMap.builder()
                        .put("mutexHandle", mutexHandle)
                        .build()));
                wiltoncall("mutex_wait", GSON.toJson(ImmutableMap.builder()
                        .put("mutexHandle", mutexHandle)
                        .put("timeoutMillis", 30000)
                        .put("conditionCallbackScript", ImmutableMap.builder()
                                .put("module", "mutex/test")
                                .put("func", "testWaitNotify")
                                .put("args", ImmutableList.of())
                                .build())
                        .build()));
                TestGateway.mutexTestShared.incrementAndGet();
                wiltoncall("mutex_unlock", GSON.toJson(ImmutableMap.builder()
                        .put("mutexHandle", mutexHandle)
                        .build()));
            }
        };
        assertEquals(-1, TestGateway.mutexTestShared.get());
        new Thread(waiter).start();
        new Thread(waiter).start();
        new Thread(waiter).start();
        assertEquals(-1, TestGateway.mutexTestShared.get());
        Thread.sleep(100);
        assertEquals(-1, TestGateway.mutexTestShared.get());
        wiltoncall("mutex_lock", GSON.toJson(ImmutableMap.builder()
                .put("mutexHandle", mutexHandle)
                .build()));
        TestGateway.mutexTestShared.set(0);
        wiltoncall("mutex_notify_all", GSON.toJson(ImmutableMap.builder()
                        .put("mutexHandle", mutexHandle)
                        .build()));
        wiltoncall("mutex_unlock", GSON.toJson(ImmutableMap.builder()
                .put("mutexHandle", mutexHandle)
                .build()));
        Thread.sleep(100);
        assertEquals(3, TestGateway.mutexTestShared.get());
        wiltoncall("mutex_destroy", GSON.toJson(ImmutableMap.builder()
                .put("mutexHandle", mutexHandle)
                .build()));
    }

    @Test
    public void testWaitCondFail() throws Exception {
        String out = wiltoncall("mutex_create");
        Map<String, Long> hamap = GSON.fromJson(out, LONG_MAP_TYPE);
        final long mutexHandle = hamap.get("mutexHandle");
        long start = System.currentTimeMillis();
        wiltoncall("mutex_lock", GSON.toJson(ImmutableMap.builder()
                .put("mutexHandle", mutexHandle)
                .build()));
        wiltoncall("mutex_wait", GSON.toJson(ImmutableMap.builder()
                .put("mutexHandle", mutexHandle)
                .put("timeoutMillis", 30000)
                .put("conditionCallbackScript", ImmutableMap.builder()
                        .put("module", "mutex/test")
                        .put("func", "testWaitCondFail")
                        .put("args", ImmutableList.of())
                        .build())
                .build()));
        wiltoncall("mutex_unlock", GSON.toJson(ImmutableMap.builder()
                .put("mutexHandle", mutexHandle)
                .build()));
        wiltoncall("mutex_destroy", GSON.toJson(ImmutableMap.builder()
                .put("mutexHandle", mutexHandle)
                .build()));
        long end = System.currentTimeMillis() - start;
        assertTrue(end - start < 1000);
    }

    @Test(expected = TestException.class)
    public void testWaitTimeoutFail() throws Exception {
        String out = wiltoncall("mutex_create");
        Map<String, Long> hamap = GSON.fromJson(out, LONG_MAP_TYPE);
        final long mutexHandle = hamap.get("mutexHandle");
        wiltoncall("mutex_lock", GSON.toJson(ImmutableMap.builder()
                .put("mutexHandle", mutexHandle)
                .build()));
        try {
            wiltoncall("mutex_wait", GSON.toJson(ImmutableMap.builder()
                    .put("mutexHandle", mutexHandle)
                    .put("timeoutMillis", 100)
                    .put("conditionCallbackScript", ImmutableMap.builder()
                            .put("module", "mutex/test")
                            .put("func", "testWaitTimeoutFail")
                            .put("args", ImmutableList.of())
                            .build())
                    .build()));
        } catch (WiltonException e) {
            throw new TestException(e);
        }
        wiltoncall("mutex_unlock", GSON.toJson(ImmutableMap.builder()
                .put("mutexHandle", mutexHandle)
                .build()));
        wiltoncall("mutex_destroy", GSON.toJson(ImmutableMap.builder()
                .put("mutexHandle", mutexHandle)
                .build()));
    }


    private static final class TestException extends RuntimeException {
        private TestException(Throwable cause) {
            super(cause);
        }
    }

}
