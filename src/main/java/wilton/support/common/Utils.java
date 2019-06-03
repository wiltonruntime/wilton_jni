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

package wilton.support.common;

import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.Closeable;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.IOException;
import java.io.Reader;
import java.io.OutputStream;
import java.io.Writer;
import java.nio.charset.Charset;
import java.util.Enumeration;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

import wilton.WiltonException;

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

    public static String readZipEntryToString(File file, String entry) {
        ZipFile zipFile = null;
        try {
            zipFile = new ZipFile(file);
        } catch (IOException e) {
            throw new WiltonException("Cannot open ZIP file, path: [" + file + "]", e);
        }
        Enumeration<? extends ZipEntry> entries = zipFile.entries();
        while (entries.hasMoreElements()) {
            ZipEntry en = entries.nextElement();
            if (entry.equals(en.getName())) {
                InputStream is = null;
                try {
                    is = zipFile.getInputStream(en);
                    ByteArrayOutputStream os = new ByteArrayOutputStream();
                    copy(is, os);
                    return new String(os.toByteArray(), Charset.forName("UTF-8"));
                } catch (IOException e) {
                    throw new RuntimeException(e);
                } finally {
                    closeQuietly(is);
                }
            }
        }
        throw new WiltonException("Cannot load entry from zip file," +
                " entry: [" + entry + "]" +
                " ZIP path: [" + file.getAbsolutePath() + "]");
    }


    public static void copy(Reader input, Writer output) throws IOException {
        char[] buffer = new char[4096];
        int n = 0;
        while (-1 != (n = input.read(buffer))) {
            output.write(buffer, 0, n);
        }
    }

    public static void copy(InputStream in, OutputStream out) throws IOException {
        byte[] buffer = new byte[4096];
        int read;
        while ((read = in.read(buffer)) != -1) {
            out.write(buffer, 0, read);
        }
    }

    public static void closeQuietly(Closeable closeable) {
        if (null != closeable) {
            try {
                closeable.close();
            } catch (IOException e) {
                // quiet
            }
        }
    }
}
