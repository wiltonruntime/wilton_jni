package utils;

import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableMap;
import net.wiltonwebtoolkit.WiltonException;
import net.wiltonwebtoolkit.WiltonGateway;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Map;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;

import static net.wiltonwebtoolkit.WiltonJni.wiltoncall;
import static utils.TestUtils.*;

/**
 * User: alexkasko
 * Date: 6/15/16
 */
public class TestGateway implements WiltonGateway {

    public static final String HELLO_RESP = "Hello Java!\n";
    public static final String ASYNC_RESP = "Hello from Async\n";
    public static final String QUERIES_RESP = "{\n" +
            "  \"foo\": \"baa,bar\",\n" +
            "  \"boo\": \"baz\"\n" +
            "}";
    public static final int TCP_PORT = 8080;
    public static final int TCP_PORT_HTTPS = 8443;
    public static final String ROOT_URL = "http://127.0.0.1:" + TCP_PORT + "/";
    public static final String ROOT_URL_HTTPS = "https://localhost:" + TCP_PORT_HTTPS + "/";
    public static final String LOG_DATA = "Please append me to log";
    public static final String STATIC_FILE_DATA = "I am data from static file";
    public static final String STATIC_ZIP_DATA = "I am data from ZIP file";
    public static final String MOCK_MODULE_PREFIX = "mock/gateway";
    public static final String MOCK_FUNC = "mock_func";

    // thread/test
    public static final CountDownLatch threadTestLatch = new CountDownLatch(1);
    public static final AtomicLong threadTestId = new AtomicLong(Thread.currentThread().getId());

    // mutex/test
    public static final AtomicInteger mutexTestShared = new AtomicInteger(-1);

    @SuppressWarnings("unchecked") // headers access
    @Override
    public String runScript(String callbackScript) throws Exception {
        Map<String, Object> csjson = GSON.fromJson(callbackScript, MAP_TYPE);
        String module = (String) csjson.get("module");
        String func = (String) csjson.get("func");
        ArrayList<Object> args = (ArrayList<Object>) csjson.get("args");
        // mock gateway impl
        if (module.startsWith(MOCK_MODULE_PREFIX)) {
            String path = module.substring(MOCK_MODULE_PREFIX.length());
            long requestHandle = (Long) args.get(0);
            try {
                String meta = wiltoncall("request_get_metadata", GSON.toJson(ImmutableMap.builder()
                        .put("requestHandle", requestHandle)
                        .build()));
                Map<String, Object> metaMap = GSON.fromJson(meta, MAP_TYPE);
                final String resp;
                if ("/hello".equalsIgnoreCase(path)) {
                    resp = HELLO_RESP;
                } else if ("/headers".equalsIgnoreCase(path)) {
                    wiltoncall("request_set_response_metadata", GSON.toJson(ImmutableMap.builder()
                            .put("requestHandle", requestHandle)
                            .put("metadata", ImmutableMap.builder()
                                    .put("headers", ImmutableMap.builder()
                                            .put("X-Server-H1", "foo")
                                            .put("X-Server-H2", "bar")
                                            .put("X-Proto", metaMap.get("protocol"))
                                            .build())
                                    .build())
                            .build()));
                    resp = GSON.toJson(metaMap.get("headers"));
                } else if ("/postmirror".equalsIgnoreCase(path)) {
                    resp = wiltoncall("request_get_data", GSON.toJson(ImmutableMap.builder()
                            .put("requestHandle", requestHandle)
                            .build()));
                } else if ("/querymirror".equalsIgnoreCase(path)) {
                    resp = GSON.toJson(metaMap.get("queries"));
                } else if ("/logger".equalsIgnoreCase(path)) {
                    String msg = wiltoncall("request_get_data", GSON.toJson(ImmutableMap.builder()
                            .put("requestHandle", requestHandle)
                            .build()));
                    String data = GSON.toJson(ImmutableMap.builder()
                            .put("level", "INFO")
                            .put("logger", TestGateway.class.getName())
                            .put("message", msg)
                            .build());
                    wiltoncall("logger_log", data);
                    resp = "";
                } else if ("/sendfile".equalsIgnoreCase(path)) {
                    String filename = wiltoncall("request_get_data", GSON.toJson(ImmutableMap.builder()
                            .put("requestHandle", requestHandle)
                            .build()));
                    wiltoncall("request_send_temp_file", GSON.toJson(ImmutableMap.builder()
                            .put("requestHandle", requestHandle)
                            .put("filePath", filename)
                            .build()));
                    resp = null;
                } else if ("/mustache".equalsIgnoreCase(path)) {
                    Map<String, String> headers = (Map<String, String>) metaMap.get("headers");
                    String mustacheFile = headers.get("X-Mustache-File");
                    String valuesJson = wiltoncall("request_get_data", GSON.toJson(ImmutableMap.builder()
                            .put("requestHandle", requestHandle)
                            .build()));
                    Map<String, Object> values = GSON.fromJson(valuesJson, MAP_TYPE);
                    wiltoncall("request_send_mustache", GSON.toJson(ImmutableMap.builder()
                            .put("requestHandle", requestHandle)
                            .put("mustacheFilePath", mustacheFile)
                            .put("values", values)
                            .build()));
                    resp = null;
                } else if ("/async".equalsIgnoreCase(path)) {
                    String out = wiltoncall("request_send_later", GSON.toJson(ImmutableMap.builder()
                            .put("requestHandle", requestHandle)
                            .build()));
                    Map<String, Long> hamap = GSON.fromJson(out, LONG_MAP_TYPE);
                    final long responseWriterHandle = hamap.get("responseWriterHandle");

                    new Thread(new Runnable() {
                        @Override
                        public void run() {
                            sleepQuietly(200);
                            wiltoncall("request_send_with_response_writer", GSON.toJson(ImmutableMap.builder()
                                    .put("responseWriterHandle", responseWriterHandle)
                                    .put("data", ASYNC_RESP)
                                    .build()));
                        }
                    }).start();
                    resp = null;
                } else if ("/reqfilename".equalsIgnoreCase(path)) {
                    resp = wiltoncall("request_get_data_filename", GSON.toJson(ImmutableMap.builder()
                            .put("requestHandle", requestHandle)
                            .build()));
                } else {
                    throw new WiltonException("Unregistered module name: [" + path + "]");
                }
                if (null != resp) { // sendfile case
                    wiltoncall("request_send_response", GSON.toJson(ImmutableMap.builder()
                            .put("requestHandle", requestHandle)
                            .put("data", resp)
                            .build()));
                }
            } catch (Throwable e) {
                e.printStackTrace();
                wiltoncall("request_send_response", GSON.toJson(ImmutableMap.builder()
                        .put("requestHandle", requestHandle)
                        .put("data", e.getMessage())
                        .build()));
            }
            return null;
        } else if (module.equals("thread/test")) {
            if (func.equals("testRun")) {
                threadTestId.set(Thread.currentThread().getId());
                threadTestLatch.countDown();
                return null;
            } else throw new WiltonException("Unknown 'thread/test' func: [" + func + "]");
        } else if (module.equals("mutex/test")) {
            if (func.equals("testWaitNotify")) {
                return GSON.toJson(ImmutableMap.builder()
                        .put("condition", mutexTestShared.get() >= 0)
                        .build());
            } else if (func.equals("testWaitCondFail")) {
                throw new IOException("Deliberate exception");
            } else if (func.equals("testWaitTimeoutFail")) {
                return GSON.toJson(ImmutableMap.builder()
                        .put("condition", false)
                        .build());
            } else throw new WiltonException("Unknown 'thread/test' func: [" + func + "]");
        } else {
            return "";
        }
    }

    public static ImmutableList<ImmutableMap<String, Object>> views() {
        return ImmutableList.<ImmutableMap<String, Object>>builder()
                .add(ImmutableMap.<String, Object>builder()
                        .put("method", "GET")
                        .put("path", "/hello")
                        .put("callbackScript", ImmutableMap.builder()
                                .put("module", MOCK_MODULE_PREFIX + "/hello")
                                .put("func", MOCK_FUNC)
                                .put("args", ImmutableList.of())
                                .build())
                        .build())
//                .add(ImmutableMap.<String, Object>builder()
//                        .put("method", "GET")
//                        .put("path", "/headers")
//                        .put("module", "/headers")
//                        .build())
//                .add(ImmutableMap.<String, Object>builder()
//                        .put("method", "POST")
//                        .put("path", "/postmirror")
//                        .put("module", "/postmirror")
//                        .build())
//                .add(ImmutableMap.<String, Object>builder()
//                        .put("method", "GET")
//                        .put("path", "/querymirror")
//                        .put("module", "/querymirror")
//                        .build())
//                .add(ImmutableMap.<String, Object>builder()
//                        .put("method", "POST")
//                        .put("path", "/logger")
//                        .put("module", "/logger")
//                        .build())
//                .add(ImmutableMap.<String, Object>builder()
//                        .put("method", "POST")
//                        .put("path", "/sendfile")
//                        .put("module", "/sendfile")
//                        .build())
//                .add(ImmutableMap.<String, Object>builder()
//                        .put("method", "POST")
//                        .put("path", "/mustache")
//                        .put("module", "/mustache")
//                        .build())
//                .add(ImmutableMap.<String, Object>builder()
//                        .put("method", "GET")
//                        .put("path", "/async")
//                        .put("module", "/async")
//                        .build())
//                .add(ImmutableMap.<String, Object>builder()
//                        .put("method", "POST")
//                        .put("path", "/reqfilename")
//                        .put("module", "/reqfilename")
//                        .build())
                .build();
    }

}
