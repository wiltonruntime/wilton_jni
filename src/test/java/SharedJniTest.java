import com.google.common.collect.ImmutableMap;
import org.junit.Test;

import static net.wiltonwebtoolkit.WiltonJni.wiltoncall;
import static org.junit.Assert.assertEquals;
import static utils.TestUtils.GSON;

/**
 * User: alexkasko
 * Date: 12/10/16
 */
public class SharedJniTest {

    @Test
    public void testPut() {
        assertEquals("", wiltoncall("shared_get", GSON.toJson(ImmutableMap.builder()
                .put("key", "foo")
                .build())));
        wiltoncall("shared_put", GSON.toJson(ImmutableMap.builder()
                .put("key", "foo")
                .put("value", "bar")
                .build()));
        assertEquals("bar", wiltoncall("shared_get", GSON.toJson(ImmutableMap.builder()
                .put("key", "foo")
                .build())));
        wiltoncall("shared_remove", GSON.toJson(ImmutableMap.builder()
                .put("key", "foo")
                .build()));
        assertEquals("", wiltoncall("shared_get", GSON.toJson(ImmutableMap.builder()
                .put("key", "foo")
                .build())));
    }

    @Test
    public void testWait() {
        wiltoncall("shared_put", GSON.toJson(ImmutableMap.builder()
                .put("key", "foo")
                .put("value", "bar")
                .build()));

        // todo

        wiltoncall("shared_remove", GSON.toJson(ImmutableMap.builder()
                .put("key", "foo")
                .build()));
    }

}
