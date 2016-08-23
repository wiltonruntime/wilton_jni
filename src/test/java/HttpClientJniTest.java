import com.google.common.collect.ImmutableMap;
import com.google.common.io.Files;
import org.apache.commons.io.FileUtils;
import org.junit.Test;
import utils.TestGateway;

import java.io.File;
import java.util.Map;

import static net.wiltonwebtoolkit.WiltonJni.*;
import static org.junit.Assert.*;
import static utils.TestGateway.*;
import static utils.TestUtils.*;

/**
 * User: alexkasko
 * Date: 6/15/16
 */
public class HttpClientJniTest {

    @Test
    public void testSimple() throws Exception {
        long handle = 0;
        long http = 0;
        try {
            handle = createServer(new TestGateway(), GSON.toJson(ImmutableMap.builder()
                    .put("tcpPort", TCP_PORT)
                    .build()));
            String out = wiltoncall("httpclient_create");
            Map<String, Long> hamap = GSON.fromJson(out, LONG_MAP_TYPE);
            http = hamap.get("httpclientHandle");
            assertTrue(http > 0);
            String resp = wiltoncall("httpclient_execute", GSON.toJson(ImmutableMap.builder()
                    .put("httpclientHandle", http)
                    .put("url", ROOT_URL)
                    .put("metadata", ImmutableMap.builder()
                            .put("forceHttp10", true)
                            .build())
                    .build()));
            Map<String, Object> map = GSON.fromJson(resp, MAP_TYPE);
            assertEquals(ROOT_RESP, map.get("data"));
        } finally {
            wiltoncall("httpclient_close", GSON.toJson(ImmutableMap.builder()
                    .put("httpclientHandle", http)
                    .build()));
            stopServer(handle);
        }
    }

    @Test
    @SuppressWarnings("unchecked") // headers map
    public void testHeaders() throws Exception {
        long handle = 0;
        long http = 0;
        try {
            handle = createServer(new TestGateway(), GSON.toJson(ImmutableMap.builder()
                    .put("tcpPort", TCP_PORT)
                    .build()));
            String out = wiltoncall("httpclient_create");
            Map<String, Long> hamap = GSON.fromJson(out, LONG_MAP_TYPE);
            http = hamap.get("httpclientHandle");
            String resp = wiltoncall("httpclient_execute", GSON.toJson(ImmutableMap.builder()
                    .put("httpclientHandle", http)
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
            wiltoncall("httpclient_close", GSON.toJson(ImmutableMap.builder()
                    .put("httpclientHandle", http)
                    .build()));
            stopServer(handle);
        }
    }

    @Test
    public void testPost() throws Exception {
        long handle = 0;
        long http = 0;
        try {
            handle = createServer(new TestGateway(), GSON.toJson(ImmutableMap.builder()
                    .put("tcpPort", TCP_PORT)
                    .build()));
            String out = wiltoncall("httpclient_create");
            Map<String, Long> hamap = GSON.fromJson(out, LONG_MAP_TYPE);
            http = hamap.get("httpclientHandle");
            String resp = wiltoncall("httpclient_execute", GSON.toJson(ImmutableMap.builder()
                    .put("httpclientHandle", http)
                    .put("url", ROOT_URL + "postmirror")
                    .put("data", "foo")
                    .put("metadata", ImmutableMap.builder()
                            .put("forceHttp10", true)
                            .build())
                    .build()));
            Map<String, Object> map = GSON.fromJson(resp, MAP_TYPE);
            assertEquals("foo", map.get("data"));
        } finally {
            wiltoncall("httpclient_close", GSON.toJson(ImmutableMap.builder()
                    .put("httpclientHandle", http)
                    .build()));
            stopServer(handle);
        }
    }

    @Test
    public void testSendFile() throws Exception {
        long handle = 0;
        long http = 0;
        File dir = null;
        try {
            dir = Files.createTempDir();
            File file = new File(dir, "test.txt");
            FileUtils.writeStringToFile(file, STATIC_FILE_DATA);
            handle = createServer(new TestGateway(), GSON.toJson(ImmutableMap.builder()
                    .put("tcpPort", TCP_PORT)
                    .build()));
            assertTrue(file.exists());
            String out = wiltoncall("httpclient_create");
            Map<String, Long> hamap = GSON.fromJson(out, LONG_MAP_TYPE);
            http = hamap.get("httpclientHandle");
            String resp = wiltoncall("httpclient_send_temp_file", GSON.toJson(ImmutableMap.builder()
                    .put("httpclientHandle", http)
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
            wiltoncall("httpclient_close", GSON.toJson(ImmutableMap.builder()
                    .put("httpclientHandle", http)
                    .build()));
            stopServer(handle);
            FileUtils.deleteDirectory(dir);
        }
    }

    @Test
    public void testHttps() throws Exception {
        long handle = 0;
        long http = 0;
        try {
            handle = createServer(new TestGateway(), GSON.toJson(ImmutableMap.builder()
                    .put("tcpPort", TCP_PORT_HTTPS)
                    .put("ssl", ImmutableMap.builder()
                            .put("keyFile", "src/test/resources/certificates/server/localhost.pem")
                            .put("keyPassword", "test")
                            .put("verifyFile", "src/test/resources/certificates/server/staticlibs_test_ca.cer")
                            .put("verifySubjectSubstr", "CN=testclient")
                            .build())
                    .build()));
            String out = wiltoncall("httpclient_create");
            Map<String, Long> hamap = GSON.fromJson(out, LONG_MAP_TYPE);
            http = hamap.get("httpclientHandle");
            String resp = wiltoncall("httpclient_execute", GSON.toJson(ImmutableMap.builder()
                    .put("httpclientHandle", http)
                    .put("url", ROOT_URL_HTTPS)
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
            assertEquals(ROOT_RESP, map.get("data"));
        } finally {
            wiltoncall("httpclient_close", GSON.toJson(ImmutableMap.builder()
                    .put("httpclientHandle", http)
                    .build()));
            stopServer(handle);
        }
    }
}
