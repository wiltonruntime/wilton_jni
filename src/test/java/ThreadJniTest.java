import com.google.common.collect.ImmutableMap;
import org.junit.Test;

import java.util.concurrent.Callable;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicLong;

import static net.wiltonwebtoolkit.WiltonJni.wiltoncall;
import static org.junit.Assert.assertTrue;
import static utils.TestUtils.GSON;

/**
 * User: alexkasko
 * Date: 12/10/16
 */
public class ThreadJniTest {

    @Test
    public void testRun() throws Exception {
        long id = Thread.currentThread().getId();
        final AtomicLong shared = new AtomicLong(id);

        final CountDownLatch latch = new CountDownLatch(1);

        wiltoncall("thread_run", "{}", new Callable<String>() {
            @Override
            public String call() {
                shared.set(Thread.currentThread().getId());
                latch.countDown();
                return "";
            }
        });
        latch.await();
        assertTrue(shared.get() != id);
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
