package utils;

import com.google.common.collect.ImmutableMap;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.reflect.TypeToken;
import net.wiltonwebtoolkit.WiltonJni;
import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.conn.scheme.Scheme;
import org.apache.http.conn.ssl.SSLConnectionSocketFactory;
import org.apache.http.conn.ssl.SSLSocketFactory;
import org.apache.http.entity.ByteArrayEntity;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.util.EntityUtils;
import org.mozilla.javascript.Context;
import org.mozilla.javascript.tools.shell.Global;

//import java.lang.RuntimeExcetion;

import javax.net.ssl.KeyManager;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;
import javax.script.ScriptEngine;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.lang.reflect.Type;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.LinkedHashMap;

import static net.wiltonwebtoolkit.WiltonJni.wiltoncall;
import static org.apache.commons.io.IOUtils.closeQuietly;

/**
 * User: alexkasko
 * Date: 5/15/16
 */
public class TestUtils {

    private static final CloseableHttpClient HTTP = HttpClients.createDefault();

    public static final Gson GSON = new GsonBuilder().setPrettyPrinting().create();
    public static final Type MAP_TYPE = new TypeToken<LinkedHashMap<String, Object>>() {}.getType();
    public static final Type STRING_MAP_TYPE = new TypeToken<LinkedHashMap<String, String>>() {}.getType();
    public static final Type LIST_MAP_TYPE = new TypeToken<ArrayList<LinkedHashMap<String, String>>>() {}.getType();
    public static final Type LONG_MAP_TYPE = new TypeToken<LinkedHashMap<String, Long>>() {}.getType();

    public static String httpGet(String url) throws Exception {
        CloseableHttpResponse resp = null;
        try {
            HttpGet get = new HttpGet(url);
            resp = HTTP.execute(get);
            return EntityUtils.toString(resp.getEntity(), "UTF-8");
        } finally {
            closeQuietly(resp);
        }
    }

    public static String httpGetHeader(String url, String header) throws Exception {
        CloseableHttpResponse resp = null;
        try {
            HttpGet get = new HttpGet(url);
            resp = HTTP.execute(get);
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
            resp = HTTP.execute(post);
            return EntityUtils.toString(resp.getEntity(), "UTF-8");
        } finally {
            closeQuietly(resp);
        }
    }

    public static int httpGetCode(String url) throws Exception {
        CloseableHttpResponse resp = null;
        try {
            HttpGet get = new HttpGet(url);
            resp = HTTP.execute(get);
            return resp.getStatusLine().getStatusCode();
        } finally {
            closeQuietly(resp);
        }
    }

    @SuppressWarnings("deprecation") // http api
    public static CloseableHttpClient createHttpsClient() {
        try {
            TrustManager[] trustAllCerts = new TrustManager[]{new NonValidatingX509TrustManager()};
            SSLContext ctx = SSLContext.getInstance("TLS");
            ctx.init(new KeyManager[]{}, trustAllCerts, null);
            SSLSocketFactory socketFactory = new SSLSocketFactory(ctx, SSLConnectionSocketFactory.ALLOW_ALL_HOSTNAME_VERIFIER);
            Scheme sch = new Scheme("https", 443, socketFactory);
            CloseableHttpClient http = new DefaultHttpClient();
            http.getConnectionManager().getSchemeRegistry().register(sch);
            return http;
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public static void sleepQuietly(long millis) {
        try {
            Thread.sleep(millis);
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }
    }

    public static void stopServerQuietly(long handle) {
        try {
            if (0 != handle) {
                wiltoncall("server_stop", GSON.toJson(ImmutableMap.builder()
                        .put("serverHandle", handle)
                        .build()));
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static class NonValidatingX509TrustManager implements X509TrustManager {
        @Override
        public void checkClientTrusted(X509Certificate[] x509Certificates, String s) throws CertificateException {
            // no-op
        }

        @Override
        public void checkServerTrusted(X509Certificate[] x509Certificates, String s) throws CertificateException {
            // no-op
        }

        @Override
        public X509Certificate[] getAcceptedIssuers() {
            return new X509Certificate[0];
        }
    }

}
