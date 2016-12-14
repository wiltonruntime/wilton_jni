import com.google.common.collect.ImmutableMap;
import org.junit.Test;
import utils.TestGateway;

import java.util.Map;

import static net.wiltonwebtoolkit.WiltonJni.wiltoncall;
import static org.junit.Assert.assertTrue;
import static utils.TestGateway.*;
import static utils.TestUtils.*;

/**
 * User: alexkasko
 * Date: 10/4/16
 */
public class MiscJniTest {

    @Test
    public void testWaitForConn() throws Exception {
        long handle = 0;
        try {
            boolean caught = false;
            try {
                wiltoncall("tcp_wait_for_connection", GSON.toJson(ImmutableMap.builder()
                        .put("ipAddress", "127.0.0.1")
                        .put("tcpPort", TCP_PORT)
                        .put("timeoutMillis", 100)
                        .build()));
            } catch (Exception e) {
                caught = true;
            }
            assertTrue(caught);

            String sout = wiltoncall("server_create", GSON.toJson(ImmutableMap.builder()
                   .put("views", TestGateway.views())
                   .put("tcpPort", TCP_PORT)
                   .build()), new TestGateway());
            Map<String, Long> shamap = GSON.fromJson(sout, LONG_MAP_TYPE);
            handle = shamap.get("serverHandle");

            wiltoncall("tcp_wait_for_connection", GSON.toJson(ImmutableMap.builder()
                    .put("ipAddress", "127.0.0.1")
                    .put("tcpPort", TCP_PORT)
                    .put("timeoutMillis", 100)
                    .build()));
        } finally {
            stopServerQuietly(handle);
        }
    }
}
