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

import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableMap;
import com.google.common.io.Files;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

import static net.wiltontoolkit.WiltonJni.LOGGING_DISABLE;
import static net.wiltontoolkit.WiltonJni.wiltoncall;
import static org.apache.commons.io.IOUtils.closeQuietly;
import static org.junit.Assert.*;
import static org.junit.Assert.assertEquals;
import static utils.TestGateway.*;
import static utils.TestUtils.*;

import org.apache.commons.io.FileUtils;
import org.apache.http.Header;
import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.util.EntityUtils;

import org.junit.BeforeClass;
import org.junit.Test;
import utils.TestGateway;

public class ServerJniTest {

    private CloseableHttpClient http = HttpClients.createDefault();

    @BeforeClass
    public static void init() {
        // init, no logging by default, enable it when needed
        initWiltonOnce(new TestGateway(), LOGGING_DISABLE, getJsDir().getAbsolutePath());
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
            assertEquals(HELLO_RESP, httpGet(ROOT_URL + "hello"));
            assertEquals("foo", httpPost(ROOT_URL + "postmirror", "foo"));
            assertEquals(404, httpGetCode(ROOT_URL + "foo"));
            // json object fields order, todo: proper comparison
            // assertEquals(QUERIES_RESP, httpGet(ROOT_URL + "querymirror?foo=bar&boo=baz&foo=baa"));
        } finally {
            stopServerQuietly(handle);
        }
    }

//    @Test // removed from server
//    public void testLogging() throws Exception {
//        File dir = null;
//        long handle = 0;
//        try {
//            dir = Files.createTempDir();
//            File logfile = new File(dir, "test.log");
//            String sout = wiltoncall("server_create", GSON.toJson(ImmutableMap.builder()
//                    .put("views", TestGateway.views())
//                    .put("tcpPort", TCP_PORT)
//                    .put("logging", ImmutableMap.builder()
//                            .put("appenders", ImmutableList.builder()
//                                    .add(ImmutableMap.builder()
//                                            .put("appenderType", "FILE")
//                                            .put("thresholdLevel", "DEBUG")
//                                            .put("filePath", logfile.getAbsolutePath())
//                                            .put("layout", "%m")
//                                            .build())
//                                    .build())
//                            .put("loggers", ImmutableList.builder()
//                                    .add(ImmutableMap.builder()
//                                            .put("name", "staticlib.httpserver")
//                                            .put("level", "WARN")
//                                            .build())
//                                    .build())
//                            .build())
//                    .build()), new TestGateway());
//            String sout = null;
//            Map<String, Long> shamap = GSON.fromJson(sout, LONG_MAP_TYPE);
//            handle = shamap.get("serverHandle");
//            assertEquals(HELLO_RESP, httpGet(ROOT_URL + "hello"));
//            httpPost(ROOT_URL + "logger", LOG_DATA);
////            todo: check me, don't flush
////            wiltoncall("logger_shutdown");
////            todo: investigateme, fails intermittently
////            assertEquals(LOG_DATA, FileUtils.readFileToString(logfile, "UTF-8"));
//        } finally {
//            stopServerQuietly(handle);
//            deleteDirQuietly(dir);
//        }
//    }

    @Test
    public void testDocumentRoot() throws Exception {
        File dir = null;
        long handle = 0;
        try {
            dir = Files.createTempDir();
            // prepare data
            FileUtils.writeStringToFile(new File(dir, "test.txt"), STATIC_FILE_DATA);
            FileUtils.writeStringToFile(new File(dir, "foo.boo"), STATIC_FILE_DATA);
            File zipFile = new File(dir, "test.zip");
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            ZipOutputStream zipper = new ZipOutputStream(baos);
            zipper.putNextEntry(new ZipEntry("test/zipped.txt"));
            zipper.write(STATIC_ZIP_DATA.getBytes("UTF-8"));
            zipper.close();
            FileUtils.writeByteArrayToFile(zipFile, baos.toByteArray());
            String sout = wiltoncall("server_create", GSON.toJson(ImmutableMap.builder()
                    .put("views", TestGateway.views())
                    .put("tcpPort", TCP_PORT)
                    .put("documentRoots", ImmutableList.builder()
                            .add(ImmutableMap.builder()
                                    .put("resource", "/static/files/")
                                    .put("dirPath", dir.getAbsolutePath())
                                    .put("mimeTypes", ImmutableList.builder()
                                            .add(ImmutableMap.builder()
                                                    .put("extension", "boo")
                                                    .put("mime", "text/x-boo")
                                                    .build())
                                            .build())
                                    .build())
                            .add(ImmutableMap.builder()
                                    .put("resource", "/static/")
                                    .put("zipPath", zipFile.getAbsolutePath())
                                    .put("zipInnerPrefix", "test/")
                                    .build())
                            .build())
                    .build()));
            Map<String, Long> shamap = GSON.fromJson(sout, LONG_MAP_TYPE);
            handle = shamap.get("serverHandle");
            assertEquals(HELLO_RESP, httpGet(ROOT_URL + "hello"));
            // deliberated repeated requests
            assertEquals(STATIC_FILE_DATA, httpGet(ROOT_URL + "static/files/test.txt"));
            assertEquals(STATIC_FILE_DATA, httpGet(ROOT_URL + "static/files/test.txt"));
            assertEquals(STATIC_FILE_DATA, httpGet(ROOT_URL + "static/files/test.txt"));
            assertEquals("text/plain", httpGetHeader(ROOT_URL + "static/files/test.txt", "Content-Type"));
            assertEquals(STATIC_FILE_DATA, httpGet(ROOT_URL + "static/files/foo.boo"));
            assertEquals("text/x-boo", httpGetHeader(ROOT_URL + "static/files/foo.boo", "Content-Type"));
            assertEquals(STATIC_ZIP_DATA, httpGet(ROOT_URL + "static/zipped.txt"));
            assertEquals(STATIC_ZIP_DATA, httpGet(ROOT_URL + "static/zipped.txt"));
            assertEquals(STATIC_ZIP_DATA, httpGet(ROOT_URL + "static/zipped.txt"));
        } finally {
            stopServerQuietly(handle);
            deleteDirQuietly(dir);
        }
    }

    // Duplicates in raw headers are handled in the following ways, depending on the header name:
    // Duplicates of age, authorization, content-length, content-type, etag, expires,
    // from, host, if-modified-since, if-unmodified-since, last-modified, location,
    // max-forwards, proxy-authorization, referer, retry-after, or user-agent are discarded.
    // For all other headers, the values are joined together with ', '.
    @Test
    public void testHeaders() throws Exception {
        long handle = 0;
        try {
            String sout = wiltoncall("server_create", GSON.toJson(ImmutableMap.builder()
                   .put("views", TestGateway.views())
                   .put("tcpPort", TCP_PORT)
                   .build()));
            Map<String, Long> shamap = GSON.fromJson(sout, LONG_MAP_TYPE);
            handle = shamap.get("serverHandle");
            assertEquals(HELLO_RESP, httpGet(ROOT_URL + "hello"));
            CloseableHttpResponse resp = null;
            String output;
            Map<String, String> serverHeaders = new LinkedHashMap<String, String>();
            try {
                HttpGet get = new HttpGet(ROOT_URL + "headers");
                get.addHeader("X-Dupl-H", "foo");
                get.addHeader("X-Dupl-H", "bar");
                get.addHeader("Referer", "foo");
                get.addHeader("referer", "bar");
                resp = http.execute(get);
                for (Header he : resp.getAllHeaders()) {
                    serverHeaders.put(he.getName(), he.getValue());
                }
                output = EntityUtils.toString(resp.getEntity(), "UTF-8");
            } finally {
                closeQuietly(resp);
            }
            Map<String, String> clientHeaders = GSON.fromJson(output, STRING_MAP_TYPE);
            assertTrue(clientHeaders.containsKey("X-Dupl-H"));
            assertTrue(clientHeaders.get("X-Dupl-H").contains("foo"));
            assertTrue(clientHeaders.get("X-Dupl-H").contains("bar"));
            assertTrue(clientHeaders.get("X-Dupl-H").contains(","));
            assertTrue(clientHeaders.containsKey("referer") || clientHeaders.containsKey("Referer"));
            if (clientHeaders.containsKey("referer")) {
                assertEquals("bar", clientHeaders.get("referer"));
            } else {
                assertEquals("foo", clientHeaders.get("Referer"));
            }
            assertEquals("foo", serverHeaders.get("X-Server-H1"));
            assertEquals("bar", serverHeaders.get("X-Server-H2"));
            assertTrue(serverHeaders.get("X-Proto").equals("http"));
        } finally {
            stopServerQuietly(handle);
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
            assertEquals(HELLO_RESP, httpGet(ROOT_URL + "hello"));
            assertTrue(file.exists());
            String contents = httpPost(ROOT_URL + "sendfile", file.getAbsolutePath());
            assertEquals(STATIC_FILE_DATA, contents);
            // todo: investigate me on windows
            //Thread.sleep(100);
            //assertFalse(file.exists());
        } finally {
            stopServerQuietly(handle);
            deleteDirQuietly(dir);
        }
    }

    @Test
    public void testMustache() throws Exception {
        String template = "{{#names}}Hi {{name}}!\n{{/names}}";
        ImmutableMap<String, Object> values = ImmutableMap.<String, Object>builder()
                .put("names", ImmutableList.builder()
                        .add(ImmutableMap.builder().put("name", "Chris").build())
                        .add(ImmutableMap.builder().put("name", "Mark").build())
                        .add(ImmutableMap.builder().put("name", "Scott").build())
                        .build())
                .build();
        ImmutableMap<String, Object> json = ImmutableMap.<String, Object>builder()
                .put("template", template)
                .put("values", values)
                .build();
        String expected = "Hi Chris!\nHi Mark!\nHi Scott!\n";

        // test mustache direct processing
        String processed = wiltoncall("mustache_render", GSON.toJson(json));
        assertEquals(expected, processed);

        // test file processing
        long handle = 0;
        File dir = null;
        CloseableHttpResponse resp = null;
        try {
            dir = Files.createTempDir();
            File file = new File(dir, "test.mustache");
            FileUtils.writeStringToFile(file, template);

            // test mustache from file processing
            String fprocessed = wiltoncall("mustache_render_file", GSON.toJson(ImmutableMap.<String, Object>builder()
                    .put("file", file.getAbsolutePath())
                    .put("values", values)
                    .build()));
            assertEquals(expected, fprocessed);

            String sout = wiltoncall("server_create", GSON.toJson(ImmutableMap.builder()
                   .put("views", TestGateway.views())
                   .put("tcpPort", TCP_PORT)
                   .build()));
            Map<String, Long> shamap = GSON.fromJson(sout, LONG_MAP_TYPE);
            handle = shamap.get("serverHandle");
            assertEquals(HELLO_RESP, httpGet(ROOT_URL + "hello"));
            assertTrue(file.exists());
            HttpPost post = new HttpPost(ROOT_URL + "mustache");
            post.setEntity(new StringEntity(GSON.toJson(json.get("values"))));
            post.addHeader("X-Mustache-File", file.getAbsolutePath());
            resp = http.execute(post);
            assertEquals(200, resp.getStatusLine().getStatusCode());
            String contents = EntityUtils.toString(resp.getEntity(), "UTF-8");
            assertEquals(expected, contents);
            Thread.sleep(200);
            assertTrue(file.exists());
        } finally {
            closeQuietly(resp);
            stopServerQuietly(handle);
            deleteDirQuietly(dir);
        }
    }

    @Test
    public void testMustachePartials() throws Exception {
        long handle = 0;
        File dir = null;
        CloseableHttpResponse resp = null;
        try {
            dir = Files.createTempDir();
            File file = new File(dir, "test.mustache");
            FileUtils.writeStringToFile(file, "<h2>Names</h2>{{#names}}{{> user}}{{/names}}");
            File partialsDir = new File(dir, "partials");
            partialsDir.mkdirs();
            File part = new File(partialsDir, "user.mustache");
            FileUtils.writeStringToFile(part, "<strong>{{name}}</strong>");
            String sout = wiltoncall("server_create", GSON.toJson(ImmutableMap.builder()
                    .put("views", TestGateway.views())
                    .put("tcpPort", TCP_PORT)
                    .put("mustache", ImmutableMap.builder()
                            .put("partialsDirs", ImmutableList.builder()
                                    .add(partialsDir.getAbsolutePath())
                                    .build())
                            .build())
                    .build()));
            Map<String, Long> shamap = GSON.fromJson(sout, LONG_MAP_TYPE);
            handle = shamap.get("serverHandle");
            assertEquals(HELLO_RESP, httpGet(ROOT_URL + "hello"));
            HttpPost post = new HttpPost(ROOT_URL + "mustache");
            post.setEntity(new StringEntity(GSON.toJson(ImmutableMap.builder()
                    .put("names", ImmutableList.builder()
                            .add(ImmutableMap.builder().put("name", "Chris").build())
                            .add(ImmutableMap.builder().put("name", "Mark").build())
                            .add(ImmutableMap.builder().put("name", "Scott").build())
                            .build())
                    .build())));
            post.addHeader("X-Mustache-File", file.getAbsolutePath());
            resp = http.execute(post);
            assertEquals(200, resp.getStatusLine().getStatusCode());
            String contents = EntityUtils.toString(resp.getEntity(), "UTF-8");
            assertEquals("<h2>Names</h2><strong>Chris</strong><strong>Mark</strong><strong>Scott</strong>", contents);
        } finally {
            closeQuietly(resp);
            stopServerQuietly(handle);
            deleteDirQuietly(dir);
        }
    }

    @Test
    public void testHttps() throws Exception {
        long handle = 0;
        CloseableHttpClient https = null;
        CloseableHttpResponse resp = null;
        try {
            String sout = wiltoncall("server_create", GSON.toJson(ImmutableMap.builder()
                    .put("views", TestGateway.views())
                    .put("tcpPort", TCP_PORT_HTTPS)
                    .put("ssl", ImmutableMap.builder()
                            .put("keyFile", "src/test/resources/certificates/server/localhost.pem")
                            .put("keyPassword", "test")
                            .build())
                    .build()));
            Map<String, Long> shamap = GSON.fromJson(sout, LONG_MAP_TYPE);
            handle = shamap.get("serverHandle");
            https = createHttpsClient();
            HttpGet get = new HttpGet(ROOT_URL_HTTPS + "hello");
            resp = https.execute(get);
            assertEquals(HELLO_RESP, EntityUtils.toString(resp.getEntity(), "UTF-8"));
        } finally {
            closeQuietly(resp);
            closeQuietly(https);
            stopServerQuietly(handle);
        }
    }

    @Test
    public void testAsync() throws Exception {
        long handle = 0;
        try {
            String sout = wiltoncall("server_create", GSON.toJson(ImmutableMap.builder()
                   .put("views", TestGateway.views())
                   .put("tcpPort", TCP_PORT)
                   .build()));
            Map<String, Long> shamap = GSON.fromJson(sout, LONG_MAP_TYPE);
            handle = shamap.get("serverHandle");
            assertEquals(HELLO_RESP, httpGet(ROOT_URL + "hello"));
            assertEquals(ASYNC_RESP, httpGet(ROOT_URL + "async"));
        } finally {
            stopServerQuietly(handle);
        }
    }

    @Test
    public void testRequestDataFile() throws Exception {
        long handle = 0;
        File dir = null;
        try {
            dir = Files.createTempDir();
            String sout = wiltoncall("server_create", GSON.toJson(ImmutableMap.builder()
                    .put("views", TestGateway.views())
                    .put("tcpPort", TCP_PORT)
                    .put("requestPayload", ImmutableMap.builder()
                            .put("tmpDirPath", dir.getAbsolutePath())
                            .put("tmpFilenameLength", 32)
                            .put("memoryLimitBytes", 4)
                            .build())
                    .build()));
            Map<String, Long> shamap = GSON.fromJson(sout, LONG_MAP_TYPE);
            handle = shamap.get("serverHandle");
            assertEquals(HELLO_RESP, httpGet(ROOT_URL + "hello"));
            { // direct writer
                String filename = httpPost(ROOT_URL + "reqfilename", "foobar");
                assertEquals(34, new File(filename).getName().length());
                sleepQuietly(100); // fs timeout
                assertFalse(new File(filename).exists());
                String posted = httpPost(ROOT_URL + "postmirror", "foobar");
                assertEquals("foobar", posted);
            }
            { // on-demand writer
                String filename = httpPost(ROOT_URL + "reqfilename", "foo");
                assertEquals(34, new File(filename).getName().length());
                sleepQuietly(100); // fs timeout
                assertFalse(new File(filename).exists());
                String posted = httpPost(ROOT_URL + "postmirror", "foo");
                assertEquals("foo", posted);
            }
        } finally {
            stopServerQuietly(handle);
            deleteDirQuietly(dir);
        }
    }

}
