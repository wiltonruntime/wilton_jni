import com.google.common.collect.ImmutableMap;
import junit.framework.Assert;
import org.junit.Test;

import java.util.Map;
import java.util.concurrent.atomic.AtomicInteger;

import static junit.framework.Assert.assertEquals;
import static net.wiltonwebtoolkit.WiltonJni.wiltoncall;
import static org.junit.Assert.assertTrue;
import static utils.TestUtils.*;

/**
 * User: alexkasko
 * Date: 9/7/16
 */
public class CronJniTest {

    @Test
    public void test() throws Exception {
        TestRunnable runnable = new TestRunnable();
        String out = wiltoncall("cron_start", GSON.toJson(ImmutableMap.builder()
                .put("expression", "* * * * * *") // every second
                .build()), runnable);
        Map<String, Long> shamap = GSON.fromJson(out, LONG_MAP_TYPE);
        long handle = shamap.get("cronHandle");
        assertEquals(0, runnable.getCount());
        // slow, uncomment for re-test
        Thread.sleep(1500);
        Assert.assertTrue(1 == runnable.getCount() || 2 == runnable.getCount());
        wiltoncall("cron_stop", GSON.toJson(ImmutableMap.builder()
                .put("cronHandle", handle)
                .build()));
        Thread.sleep(1000);
        assertTrue(2 == runnable.getCount() || 3 == runnable.getCount());
    }

    private static class TestRunnable implements Runnable {
        AtomicInteger counter = new AtomicInteger(0);

        @Override
        public void run() {
            counter.incrementAndGet();
        }

        int getCount() {
            return counter.get();
        }
    }

}
