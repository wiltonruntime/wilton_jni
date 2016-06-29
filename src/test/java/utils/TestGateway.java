package utils;

import com.google.common.collect.ImmutableMap;
import net.wiltonwebtoolkit.WiltonGateway;

import java.util.Map;

import static net.wiltonwebtoolkit.WiltonJni.*;
import static net.wiltonwebtoolkit.WiltonJni.sendResponse;
import static net.wiltonwebtoolkit.WiltonJni.setResponseMetadata;
import static utils.TestUtils.GSON;
import static utils.TestUtils.MAP_TYPE;
import static utils.TestUtils.sleepQuietly;

/**
 * User: alexkasko
 * Date: 6/15/16
 */
public class TestGateway implements WiltonGateway {

    public static final String ROOT_RESP = "Hello Java!\n";
    public static final String NOT_FOUND_RESP = "Not found\n";
    public static final String ASYNC_RESP = "Hello from Async\n";
    public static final int TCP_PORT = 8080;
    public static final int TCP_PORT_HTTPS = 8443;
    public static final String ROOT_URL = "http://127.0.0.1:" + TCP_PORT + "/";
    public static final String ROOT_URL_HTTPS = "https://127.0.0.1:" + TCP_PORT_HTTPS + "/";
    public static final String LOG_DATA = "Please append me to log";
    public static final String STATIC_FILE_DATA = "I am data from static file";
    public static final String STATIC_ZIP_DATA = "I am data from ZIP file";

    @SuppressWarnings("unchecked") // headers access
    @Override
    public void gatewayCallback(long requestHandle) {
        try {
            String meta = getRequestMetadata(requestHandle);
            Map<String, Object> metaMap = GSON.fromJson(meta, MAP_TYPE);
            String path = String.valueOf(metaMap.get("pathname"));
            final String resp;
            if ("/".equalsIgnoreCase(path)) {
                resp = ROOT_RESP;
            } else if ("/headers".equalsIgnoreCase(path)) {
                String json = GSON.toJson(ImmutableMap.builder()
                        .put("headers", ImmutableMap.builder()
                                .put("X-Server-H1", "foo")
                                .put("X-Server-H2", "bar")
                                .put("X-Proto", metaMap.get("protocol"))
                                .build())
                        .build());
                setResponseMetadata(requestHandle, json);
                resp = GSON.toJson(metaMap.get("headers"));
            } else if ("/postmirror".equalsIgnoreCase(path)) {
                resp = getRequestData(requestHandle);
            } else if ("/logger".equalsIgnoreCase(path)) {
                String data = getRequestData(requestHandle);
                appendLog("INFO", TestGateway.class.getName(), data);
                resp = "";
            } else if ("/sendfile".equalsIgnoreCase(path)) {
                String filename = getRequestData(requestHandle);
                sendTempFile(requestHandle, filename);
                resp = null;
            } else if ("/mustache".equalsIgnoreCase(path)) {
                Map<String, String> headers = (Map<String, String>) metaMap.get("headers");
                String mustacheFile = headers.get("X-Mustache-File");
                String values = getRequestData(requestHandle);
                sendMustache(requestHandle, mustacheFile, values);
                resp = null;
            } else if ("/async".equalsIgnoreCase(path)) {
                final long responseWriterHandle = sendLater(requestHandle);
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        sleepQuietly(200);
                        sendWithResponseWriter(responseWriterHandle, ASYNC_RESP);
                    }
                }).start();
                resp = null;
            } else if ("/reqfilename".equalsIgnoreCase(path)) {
                resp = getRequestDataFilename(requestHandle);
            } else {
                String json = GSON.toJson(ImmutableMap.builder()
                        .put("statusCode", 404)
                        .put("statusMessage", "Not Found")
                        .build());
                setResponseMetadata(requestHandle, json);
                resp = NOT_FOUND_RESP;
            }
            if (null != resp) { // sendfile case
                sendResponse(requestHandle, resp);
            }
        } catch (Throwable e) {
            e.printStackTrace();
            sendResponse(requestHandle, e.getMessage());
        }
    }
}
