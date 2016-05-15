package utils;

import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.entity.ByteArrayEntity;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.util.EntityUtils;

import static org.apache.commons.io.IOUtils.closeQuietly;

/**
 * User: alexkasko
 * Date: 5/15/16
 */
public class TestUtils {

    private static final CloseableHttpClient http = HttpClients.createDefault();

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
}
