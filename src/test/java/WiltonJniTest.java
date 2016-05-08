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
import java.util.LinkedHashMap;
import java.util.Map;
import net.wiltonwebtoolkit.HttpGateway;
import static net.wiltonwebtoolkit.HttpServerJni.*;
import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.util.EntityUtils;
import static org.junit.Assert.assertEquals;
import org.junit.Test;

public class WiltonJniTest {
    
    private static Gson GSON = new Gson();
    private static int TCP_PORT = 8080;
    private static String ROOT_URL = "http://127.0.0.1:" + TCP_PORT + "/";
    private static String ROOT_RESP = "Hello Java!\n";
    
    private CloseableHttpClient http = HttpClients.createDefault();

    private static class TestGateway implements HttpGateway {
        @Override
        public void gatewayCallback(long requestHandle) {
//            System.out.println(getRequestMetadata(requestHandle));
            sendResponse(requestHandle, ROOT_RESP);
        }
    }
    
    @Test
    public void test() throws Exception {
        long handle = 0;
        try {
            createServer(new TestGateway(), serverConfig());
            assertEquals(ROOT_URL, ROOT_RESP, httpGet(ROOT_URL));
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
            HttpGet httpGet = new HttpGet(url);
            resp = http.execute(httpGet);
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
