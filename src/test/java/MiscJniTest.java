import com.google.common.collect.ImmutableMap;
import net.wiltontoolkit.WiltonException;
import org.junit.BeforeClass;
import org.junit.Test;
import utils.TestGateway;

import java.util.Map;

import static net.wiltontoolkit.WiltonJni.*;
import static org.junit.Assert.assertTrue;
import static utils.TestGateway.*;
import static utils.TestUtils.*;

/**
 * User: alexkasko
 * Date: 10/4/16
 */
public class MiscJniTest {

    @BeforeClass
    public static void init() {
        // init, no logging by default, enable it when needed
        initWiltonOnce(new TestGateway(), LOGGING_DISABLE, getJsDir().getAbsolutePath());
    }

    @Test
    public void testWaitForConn() throws Exception {
        long handle = 0;
        try {
            // check error
            boolean caught = false;
            try {
                wiltoncall("tcp_wait_for_connection", GSON.toJson(ImmutableMap.builder()
                        .put("ipAddress", "127.0.0.1")
                        .put("tcpPort", TCP_PORT)
                        .put("timeoutMillis", 100)
                        .build()));
            } catch (WiltonException e) {
                caught = true;
            }
            assertTrue(caught);

            // check success
            String sout = wiltoncall("server_create", GSON.toJson(ImmutableMap.builder()
                   .put("views", TestGateway.views())
                   .put("tcpPort", TCP_PORT)
                   .build()));
            Map<String, Long> shamap = GSON.fromJson(sout, LONG_MAP_TYPE);
            handle = shamap.get("serverHandle");

            wiltoncall("net_wait_for_tcp_connection", GSON.toJson(ImmutableMap.builder()
                    .put("ipAddress", "127.0.0.1")
                    .put("tcpPort", TCP_PORT)
                    .put("timeoutMillis", 100)
                    .build()));

        } finally {
            stopServerQuietly(handle);
        }
    }
}
