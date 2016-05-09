/*
 * Copyright 2016, alex at staticlibs.net
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import com.google.gson.Gson;
import java.io.Closeable;
import java.lang.reflect.Type;
import java.util.LinkedHashMap;
import java.util.Map;

import com.google.gson.reflect.TypeToken;
import net.wiltonwebtoolkit.HttpGateway;
import static net.wiltonwebtoolkit.HttpServerJni.*;
import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.entity.ByteArrayEntity;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.util.EntityUtils;
import static org.junit.Assert.assertEquals;
import org.junit.Test;

public class WiltonJniTest {
    
    private static final Gson GSON = new Gson();
    private static final Type MAP_TYPE = new TypeToken<LinkedHashMap<String, Object>>() {}.getType();
    private static final int TCP_PORT = 8080;
    private static final String ROOT_URL = "http://127.0.0.1:" + TCP_PORT + "/";
    private static final String ROOT_RESP = "Hello Java!\n";
    private static final String NOT_FOUND_RESP = "Not found\n";

    private CloseableHttpClient http = HttpClients.createDefault();

    private static class TestGateway implements HttpGateway {
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
                    resp = String.valueOf(metaMap.get("headers"));
                } else if ("/postmirror".equalsIgnoreCase(path)) {
                    resp = getRequestData(requestHandle);
                } else {
                    Map<String, Object> map = new LinkedHashMap<String, Object>();
                    map.put("statusCode", 404);
                    map.put("statusMessage", "Not Found");
                    setResponseMetadata(requestHandle, GSON.toJson(map));
                    resp = NOT_FOUND_RESP;
                }
                sendResponse(requestHandle, resp);
            } catch (Throwable e) {
                e.printStackTrace();
                sendResponse(requestHandle, e.getMessage());
            }
        }
    }
    
    @Test
    public void test() throws Exception {
        long handle = 0;
        try {
            handle = createServer(new TestGateway(), serverConfig());
            assertEquals(ROOT_RESP, httpGet(ROOT_URL));
            assertEquals("foo", httpPost(ROOT_URL + "postmirror", "foo"));
            assertEquals(NOT_FOUND_RESP, httpGet(ROOT_URL + "foo"));
            assertEquals(404, httpGetCode(ROOT_URL + "foo"));
        } finally {
            stopServer(handle);
        }
    }
    
    private String serverConfig() {
        Map<String, Object> config = new LinkedHashMap<String, Object>();
        config.put("tcpPort", TCP_PORT);
        return GSON.toJson(config);
    }
    
    private String httpGet(String url) throws Exception {
        CloseableHttpResponse resp = null;
        try {
            HttpGet get = new HttpGet(url);
            resp = http.execute(get);
            return EntityUtils.toString(resp.getEntity(), "UTF-8");
        } finally {
            closeQuietly(resp);
        }
    }

    private int httpGetCode(String url) throws Exception {
        CloseableHttpResponse resp = null;
        try {
            HttpGet get = new HttpGet(url);
            resp = http.execute(get);
            return resp.getStatusLine().getStatusCode();
        } finally {
            closeQuietly(resp);
        }
    }

    private String httpPost(String url, String data) throws Exception {
        CloseableHttpResponse resp = null;
        try {
            HttpPost post = new HttpPost(url);
            post.setEntity(new ByteArrayEntity(data.getBytes("UTF-8")));
            resp = http.execute(post);
            return EntityUtils.toString(resp.getEntity(), "UTF-8");
        } finally {
            closeQuietly(resp);
        }
    }
    
    static void closeQuietly(Closeable closeable) {
        if (null == closeable) {
            return;
        }
        try {
            closeable.close();
        } catch (Exception e) {
            // ignore
        }
    }

}
