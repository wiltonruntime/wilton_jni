import com.google.common.collect.ImmutableMap;
import org.junit.Test;

import java.util.Map;

import static net.wiltonwebtoolkit.WiltonJni.wiltoncall;
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
    public void test() throws Exception {
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

}
