package net.wiltontoolkit.support.common;

import java.io.*;

/**
 * User: alexkasko
 * Date: 2/11/17
 */
public class Utils {

    public static String readFileToString(File file) throws IOException {
        FileInputStream is = null;
        try {
            is = new FileInputStream(file);
            StringBuilderWriter writer = new StringBuilderWriter();
            Reader reader = new BufferedReader(new InputStreamReader(is, "UTF-8"));
            copy(reader, writer);
            return writer.toString();
        } finally {
            closeQuietly(is);
        }
    }

    private static void copy(Reader input, Writer output) throws IOException {
        char[] buffer = new char[4096];
        int n = 0;
        while (-1 != (n = input.read(buffer))) {
            output.write(buffer, 0, n);
        }
    }

    private static void closeQuietly(Closeable closeable) {
        if (null != closeable) {
            try {
                closeable.close();
            } catch (IOException e) {
                // quiet
            }
        }
    }
}
