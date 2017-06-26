import com.google.common.collect.ImmutableMap;
import com.google.common.io.Files;
import org.apache.commons.io.FileUtils;
import org.junit.BeforeClass;
import org.junit.Test;
import utils.TestGateway;

import java.io.File;
import java.util.Map;

import static net.wiltontoolkit.WiltonJni.LOGGING_DISABLE;
import static net.wiltontoolkit.WiltonJni.wiltoncall;
import static org.junit.Assert.*;
import static utils.TestGateway.*;
import static utils.TestUtils.*;

/**
 * User: alexkasko
 * Date: 6/15/16
 */
public class HttpClientJniTest {

    @BeforeClass
    public static void init() {
        // init, no logging by default, enable it when needed
        initWiltonOnce(new TestGateway(), LOGGING_DISABLE);
    }

    @Test
    public void testSimple() throws Exception {
        long handle = 0;
        try {
             String sout = wiltoncall("server_create", GSON.toJson(ImmutableMap.builder()
                     .put("views", TestGateway.views())
                     .put("tcpPort", TCP_PORT)
                     .build()));
            Map<String, Long> shamap = GSON.fromJson(sout, LONG_MAP_TYPE);
            handle = shamap.get("serverHandle");
            String resp = wiltoncall("httpclient_send_request", GSON.toJson(ImmutableMap.builder()
                    .put("url", ROOT_URL + "hello")
                    .put("metadata", ImmutableMap.builder()
                            .put("forceHttp10", true)
                            .build())
                    .build()));
            Map<String, Object> map = GSON.fromJson(resp, MAP_TYPE);
            assertEquals(HELLO_RESP, map.get("data"));
        } finally {
            wiltoncall("server_stop", GSON.toJson(ImmutableMap.builder()
                    .put("serverHandle", handle)
                    .build()));
        }
    }

    @Test
    @SuppressWarnings("unchecked") // headers map
    public void testHeaders() throws Exception {
        long handle = 0;
        try {
            String sout = wiltoncall("server_create", GSON.toJson(ImmutableMap.builder()
                   .put("views", TestGateway.views())
                   .put("tcpPort", TCP_PORT)
                   .build()));
            Map<String, Long> shamap = GSON.fromJson(sout, LONG_MAP_TYPE);
            handle = shamap.get("serverHandle");
            String resp = wiltoncall("httpclient_send_request", GSON.toJson(ImmutableMap.builder()
                    .put("url", ROOT_URL + "headers")
                    .put("metadata", ImmutableMap.builder()
                            .put("forceHttp10", true)
                            .put("headers", ImmutableMap.builder()
                                    .put("X-Foo", "bar")
                                    .build())
                            .build())
                    .build()));
            Map<String, Object> map = GSON.fromJson(resp, MAP_TYPE);
            assertNotNull(map.get("headers"));
            Map<String, Object> headers = (Map<String, Object>) map.get("headers");
            assertEquals("foo", headers.get("X-Server-H1"));
        } finally {
            wiltoncall("server_stop", GSON.toJson(ImmutableMap.builder()
                    .put("serverHandle", handle)
                    .build()));
        }
    }

    @Test
    public void testPost() throws Exception {
        long handle = 0;
        try {
            String sout = wiltoncall("server_create", GSON.toJson(ImmutableMap.builder()
                   .put("views", TestGateway.views())
                   .put("tcpPort", TCP_PORT)
                   .build()));
            Map<String, Long> shamap = GSON.fromJson(sout, LONG_MAP_TYPE);
            handle = shamap.get("serverHandle");
            String resp = wiltoncall("httpclient_send_request", GSON.toJson(ImmutableMap.builder()
                    .put("url", ROOT_URL + "postmirror")
                    .put("data", "foo")
                    .put("metadata", ImmutableMap.builder()
                            .put("forceHttp10", true)
                            .build())
                    .build()));
            Map<String, Object> map = GSON.fromJson(resp, MAP_TYPE);
            assertEquals("foo", map.get("data"));
        } finally {
            wiltoncall("server_stop", GSON.toJson(ImmutableMap.builder()
                    .put("serverHandle", handle)
                    .build()));
        }
    }

    @Test
    public void testSendFile() throws Exception {
        long handle = 0;
        File dir = null;
        try {
            dir = Files.createTempDir();
            File file = new File(dir, "test.txt");
            FileUtils.writeStringToFile(file, STATIC_FILE_DATA);
            String sout = wiltoncall("server_create", GSON.toJson(ImmutableMap.builder()
                   .put("views", TestGateway.views())
                   .put("tcpPort", TCP_PORT)
                   .build()));
            Map<String, Long> shamap = GSON.fromJson(sout, LONG_MAP_TYPE);
            handle = shamap.get("serverHandle");
            assertTrue(file.exists());
            String resp = wiltoncall("httpclient_send_temp_file", GSON.toJson(ImmutableMap.builder()
                    .put("url", ROOT_URL + "postmirror")
                    .put("filePath", file.getAbsolutePath())
                    .put("metadata", ImmutableMap.builder()
                            .put("forceHttp10", true)
                            .build())
                    .build()));
            Map<String, Object> map = GSON.fromJson(resp, MAP_TYPE);
            assertEquals(STATIC_FILE_DATA, map.get("data"));
            Thread.sleep(200);
            assertFalse(file.exists());
        } finally {
            wiltoncall("server_stop", GSON.toJson(ImmutableMap.builder()
                    .put("serverHandle", handle)
                    .build()));
            deleteDirQuietly(dir);
        }
    }

    @Test
    public void testHttps() throws Exception {
        long handle = 0;
        try {
            String sout = wiltoncall("server_create", GSON.toJson(ImmutableMap.builder()
                    .put("views", TestGateway.views())
                    .put("tcpPort", TCP_PORT_HTTPS)
                    .put("ssl", ImmutableMap.builder()
                            .put("keyFile", "src/test/resources/certificates/server/localhost.pem")
                            .put("keyPassword", "test")
                            .put("verifyFile", "src/test/resources/certificates/server/staticlibs_test_ca.cer")
                            .put("verifySubjectSubstr", "CN=testclient")
                            .build())
                    .build()));
            Map<String, Long> shamap = GSON.fromJson(sout, LONG_MAP_TYPE);
            handle = shamap.get("serverHandle");
            String resp = wiltoncall("httpclient_send_request", GSON.toJson(ImmutableMap.builder()
                    .put("url", ROOT_URL_HTTPS + "hello")
                    .put("metadata", ImmutableMap.builder()
                            .put("forceHttp10", true)
                            .put("sslcertFilename", "src/test/resources/certificates/client/testclient.cer")
                            .put("sslcertype", "PEM")
                            .put("sslkeyFilename", "src/test/resources/certificates/client/testclient.key")
                            .put("sslKeyType", "PEM")
                            .put("sslKeypasswd", "test")
                            .put("sslVerifyhost", true)
                            .put("sslVerifypeer", true)
                            .put("cainfoFilename", "src/test/resources/certificates/client/staticlibs_test_ca.cer")
                            .build())
                    .build()));
            Map<String, Object> map = GSON.fromJson(resp, MAP_TYPE);
            assertEquals(HELLO_RESP, map.get("data"));
        } finally {
            wiltoncall("server_stop", GSON.toJson(ImmutableMap.builder()
                    .put("serverHandle", handle)
                    .build()));
        }
    }

    @Test
    public void testConnectFail() throws Exception {
        String resp = wiltoncall("httpclient_send_request", GSON.toJson(ImmutableMap.builder()
                .put("url", ROOT_URL + "hello")
                .put("metadata", ImmutableMap.builder()
                        .put("forceHttp10", true)
                        .put("connecttimeoutMillis", 100)
                        .put("abortOnConnectError", false)
                        .build())
                .build()));
        Map<String, Object> map = GSON.fromJson(resp, MAP_TYPE);
        assertEquals(false, map.get("connectionSuccess"));
    }
}
