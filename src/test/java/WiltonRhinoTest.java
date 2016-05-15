import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.entity.ByteArrayEntity;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.util.EntityUtils;
import org.junit.Test;
import org.mozilla.javascript.Context;
import org.mozilla.javascript.tools.shell.Global;

import java.io.FileInputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;

import static org.apache.commons.io.IOUtils.closeQuietly;

/**
 * User: alexkasko
 * Date: 5/15/16
 */
public class WiltonRhinoTest {

    private static CloseableHttpClient http = HttpClients.createDefault();

    @Test
    public void test() throws Exception {
        Context cx = Context.enter();
        cx.setOptimizationLevel(-1);
        Global gl = new Global();
        gl.init(cx);
        {
            InputStream is = null;
            try {
                is = new FileInputStream("js/wilton.js");
                Reader re = new InputStreamReader(is, "UTF-8");
                cx.evaluateReader(gl, re, "wilton.js", -1, null);
            } finally {
                closeQuietly(is);
            }
        }
        {
            InputStream is = null;
            try {
                is = WiltonRhinoTest.class.getResourceAsStream("/wilton_test.js");
                Reader re = new InputStreamReader(is, "UTF-8");
                cx.evaluateReader(gl, re, "wilton_test.js", -1, null);
            } finally {
                closeQuietly(is);
            }
        }
    }

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
}
