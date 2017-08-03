package utils;

import com.google.common.collect.ImmutableMap;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.reflect.TypeToken;
import net.wiltontoolkit.WiltonGateway;
import org.apache.commons.io.FileUtils;
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

//import java.lang.RuntimeExcetion;

import javax.net.ssl.KeyManager;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;
import java.io.File;
import java.lang.reflect.Type;
import java.net.URI;
import java.net.URISyntaxException;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.concurrent.atomic.AtomicBoolean;

import static net.wiltontoolkit.WiltonJni.wiltoncall;
import static net.wiltontoolkit.WiltonJni.wiltoninit;
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
    public static final AtomicBoolean INITTED = new AtomicBoolean(false);
    public static WiltonGateway GATEWAY;

    public static void initWiltonOnce(WiltonGateway gateway, String loggingConf, String pathToWiltonDir) {
        if (INITTED.compareAndSet(false, true)) {
            String jsPath = "file://" + new File(pathToWiltonDir, "js").getAbsolutePath() + File.separator;

            String config = GSON.toJson(ImmutableMap.builder()
                    .put("defaultScriptEngine", "jni")
                    .put("applicationDirectory", "UNSPECIFIED")
                    .put("requireJs", ImmutableMap.builder()
                            .put("waitSeconds", 0)
                            .put("enforceDefine", true)
                            .put("nodeIdCompat", true)
                            .put("baseUrl", jsPath)
                            .put("paths", ImmutableMap.builder()
                                    .put("test/scripts", jsPath + "../core/test/scripts")
                                    .build())
                            .build())
                    .build());

            wiltoninit(gateway, config);
            wiltoncall("logging_initialize", loggingConf);
            GATEWAY = gateway;
        }
    }

    public static void deleteDirQuietly(File dir) {
        try {
            if (null != dir) {
                FileUtils.deleteDirectory(dir);
            }
        } catch (Exception e) {
//            e.printStackTrace();
        }
    }

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

    public static File getJsDir() {
        File testClasses = codeSourceDir(TestUtils.class);
        File project = testClasses.getParentFile().getParentFile();
        return new File(project, "..");
    }

    // points to <project>/target/test-classes
    private static File codeSourceDir(Class<?> clazz) {
        URI uri = null;
        try {
            uri = clazz.getProtectionDomain().getCodeSource().getLocation().toURI();
            File jarOrDir = new File(uri);
            return jarOrDir.isDirectory() ? jarOrDir : jarOrDir.getParentFile();
        } catch (URISyntaxException e) {
            throw new RuntimeException(e);
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
