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
