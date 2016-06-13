package utils;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.reflect.TypeToken;
import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.entity.ByteArrayEntity;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.util.EntityUtils;
import org.mozilla.javascript.Context;
import org.mozilla.javascript.tools.shell.Global;

//import java.lang.RuntimeExcetion;

import javax.script.ScriptEngine;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.lang.reflect.Type;
import java.util.ArrayList;
import java.util.LinkedHashMap;

import static org.apache.commons.io.IOUtils.closeQuietly;

/**
 * User: alexkasko
 * Date: 5/15/16
 */
public class TestUtils {

    private static final CloseableHttpClient http = HttpClients.createDefault();

    public static final Gson GSON = new GsonBuilder().setPrettyPrinting().create();
    public static final Type MAP_TYPE = new TypeToken<LinkedHashMap<String, Object>>() {}.getType();
    public static final Type STRING_MAP_TYPE = new TypeToken<LinkedHashMap<String, String>>() {}.getType();
    public static final Type LIST_MAP_TYPE = new TypeToken<ArrayList<LinkedHashMap<String, String>>>() {}.getType();

    public static String httpGet(String url) throws Exception {
        CloseableHttpResponse resp = null;
        try {
            HttpGet get = new HttpGet(url);
            resp = http.execute(get);
            return EntityUtils.toString(resp.getEntity(), "UTF-8");
        } finally {
            closeQuietly(resp);
        }
    }

    public static String httpGetHeader(String url, String header) throws Exception {
        CloseableHttpResponse resp = null;
        try {
            HttpGet get = new HttpGet(url);
            resp = http.execute(get);
            return resp.getFirstHeader(header).getValue();
        } finally {
            closeQuietly(resp);
        }
    }

    public static String httpPost(String url, String data) throws Exception {
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

    public static int httpGetCode(String url) throws Exception {
        CloseableHttpResponse resp = null;
        try {
            HttpGet get = new HttpGet(url);
            resp = http.execute(get);
            return resp.getStatusLine().getStatusCode();
        } finally {
            closeQuietly(resp);
        }
    }

    public static void runRhinoFs(Context cx, Global gl, File file) {
        InputStream is = null;
        try {
            is = new FileInputStream(file.getAbsolutePath());
            Reader re = new InputStreamReader(is, "UTF-8");
            cx.evaluateReader(gl, re, file.getName(), -1, null);
        } catch(Exception e) {
            throw new RuntimeException(e);
        } finally {
            closeQuietly(is);
        }
    }

    public static void runRhinoClasspath(Context cx, Global gl, String path) {
        InputStream is = null;
        try {
            is = TestUtils.class.getResourceAsStream(path);
            Reader re = new InputStreamReader(is, "UTF-8");
            cx.evaluateReader(gl, re, path, -1, null);
        } catch(Exception e) {
            throw new RuntimeException(e);
        } finally {
            closeQuietly(is);
        }
    }

    public static void runNashornFs(ScriptEngine engine, File file) {
        InputStream is = null;
        try {
            is = new FileInputStream(file.getAbsolutePath());
            Reader re = new InputStreamReader(is, "UTF-8");
            engine.eval(re);
        } catch(Exception e) {
            throw new RuntimeException(e);
        } finally {
            closeQuietly(is);
        }
    }

    public static void runNashornClasspath(ScriptEngine engine, String path) {
        InputStream is = null;
        try {
            is = TestUtils.class.getResourceAsStream(path);
            Reader re = new InputStreamReader(is, "UTF-8");
            engine.eval(re);
        } catch(Exception e) {
            throw new RuntimeException(e);
        } finally {
            closeQuietly(is);
        }
    }




}
