import com.google.common.collect.ImmutableMap;
import net.wiltonwebtoolkit.WiltonException;
import org.junit.Test;

import java.util.Map;
import java.util.concurrent.Callable;

import static net.wiltonwebtoolkit.WiltonJni.wiltoncall;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static utils.TestUtils.GSON;
import static utils.TestUtils.LONG_MAP_TYPE;

/**
 * User: alexkasko
 * Date: 10/4/16
 */
public class MutexJniTest {

    @Test
    public void testLock() throws Exception {
        String out = wiltoncall("mutex_create");
        Map<String, Long> hamap = GSON.fromJson(out, LONG_MAP_TYPE);
        final long mutexHandle = hamap.get("mutexHandle");
        wiltoncall("mutex_lock", GSON.toJson(ImmutableMap.builder()
                .put("mutexHandle", mutexHandle)
                .build()));
        final boolean [] shared = new boolean[]{false};
        new Thread(new Runnable() {
            @Override
            public void run() {
                wiltoncall("mutex_lock", GSON.toJson(ImmutableMap.builder()
                        .put("mutexHandle", mutexHandle)
                        .build()));
                shared[0] = true;
                wiltoncall("mutex_unlock", GSON.toJson(ImmutableMap.builder()
                        .put("mutexHandle", mutexHandle)
                        .build()));
            }
        }).start();
        assertFalse(shared[0]);
        wiltoncall("mutex_unlock", GSON.toJson(ImmutableMap.builder()
                .put("mutexHandle", mutexHandle)
                .build()));
        Thread.sleep(100);
        assertTrue(shared[0]);
        wiltoncall("mutex_destroy", GSON.toJson(ImmutableMap.builder()
                .put("mutexHandle", mutexHandle)
                .build()));
    }

    @Test
    public void testWaitNotify() throws Exception {
        String out = wiltoncall("mutex_create");
        Map<String, Long> hamap = GSON.fromJson(out, LONG_MAP_TYPE);
        final long mutexHandle = hamap.get("mutexHandle");
        final int [] shared = new int[]{-1};
        final Callable<String> cond = new Callable<String>() {
            @Override
            public String call() throws Exception {
                return GSON.toJson(ImmutableMap.builder()
                        .put("condition", shared[0] >= 0)
                        .build());
            }
        };
        Runnable waiter = new Runnable() {
            @Override
            public void run() {
                wiltoncall("mutex_lock", GSON.toJson(ImmutableMap.builder()
                        .put("mutexHandle", mutexHandle)
                        .build()));
                wiltoncall("mutex_wait", GSON.toJson(ImmutableMap.builder()
                        .put("mutexHandle", mutexHandle)
                        .put("timeoutMillis", 30000)
                        .build()), cond);
                shared[0] = shared[0] + 1;
                wiltoncall("mutex_unlock", GSON.toJson(ImmutableMap.builder()
                        .put("mutexHandle", mutexHandle)
                        .build()));
            }
        };
        assertEquals(-1, shared[0]);
        new Thread(waiter).start();
        new Thread(waiter).start();
        new Thread(waiter).start();
        assertEquals(-1, shared[0]);
        Thread.sleep(100);
        assertEquals(-1, shared[0]);
        wiltoncall("mutex_lock", GSON.toJson(ImmutableMap.builder()
                .put("mutexHandle", mutexHandle)
                .build()));
        shared[0] = 0;
        wiltoncall("mutex_notify_all", GSON.toJson(ImmutableMap.builder()
                        .put("mutexHandle", mutexHandle)
                        .build()));
        wiltoncall("mutex_unlock", GSON.toJson(ImmutableMap.builder()
                .put("mutexHandle", mutexHandle)
                .build()));
        Thread.sleep(100);
        assertEquals(3, shared[0]);
        wiltoncall("mutex_destroy", GSON.toJson(ImmutableMap.builder()
                .put("mutexHandle", mutexHandle)
                .build()));
    }

    @Test(expected = TestException.class)
    public void testWaitCondFail() throws Exception {
        String out = wiltoncall("mutex_create");
        Map<String, Long> hamap = GSON.fromJson(out, LONG_MAP_TYPE);
        final long mutexHandle = hamap.get("mutexHandle");
        final Callable<String> cond = new Callable<String>() {
            @Override
            public String call() throws Exception {
                throw new TestException("Deliberate exception");
            }
        };
        wiltoncall("mutex_lock", GSON.toJson(ImmutableMap.builder()
                .put("mutexHandle", mutexHandle)
                .build()));
        wiltoncall("mutex_wait", GSON.toJson(ImmutableMap.builder()
                .put("mutexHandle", mutexHandle)
                .put("timeoutMillis", 30000)
                .build()), cond);
        wiltoncall("mutex_unlock", GSON.toJson(ImmutableMap.builder()
                .put("mutexHandle", mutexHandle)
                .build()));
        wiltoncall("mutex_destroy", GSON.toJson(ImmutableMap.builder()
                .put("mutexHandle", mutexHandle)
                .build()));

    }

    @Test(expected = TestException.class)
    public void testWaitTimeoutFail() throws Exception {
        String out = wiltoncall("mutex_create");
        Map<String, Long> hamap = GSON.fromJson(out, LONG_MAP_TYPE);
        final long mutexHandle = hamap.get("mutexHandle");
        final Callable<String> cond = new Callable<String>() {
            @Override
            public String call() throws Exception {
                return GSON.toJson(ImmutableMap.builder()
                        .put("condition", false)
                        .build());
            }
        };
        wiltoncall("mutex_lock", GSON.toJson(ImmutableMap.builder()
                .put("mutexHandle", mutexHandle)
                .build()));
        try {
            wiltoncall("mutex_wait", GSON.toJson(ImmutableMap.builder()
                    .put("mutexHandle", mutexHandle)
                    .put("timeoutMillis", 100)
                    .build()), cond);
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
        private TestException(String message) {
            super(message);
        }

        private TestException(Throwable cause) {
            super(cause);
        }
    }

}
