package net.wiltonwebtoolkit.support.rhino;

import org.mozilla.javascript.Context;
import org.mozilla.javascript.Function;
import org.mozilla.javascript.Script;
import org.mozilla.javascript.Scriptable;

import java.io.*;
import java.lang.reflect.Method;

/**
 * User: alexkasko
 * Date: 2/11/17
 */
class WiltonRhinoScriptLoader {

    public static void load(Context cx, Scriptable thisObj, Object[] args, Function funObj) throws Exception {
        WiltonRhinoEnvironment.checkInitialized();
        for (Object arg : args) {
            String filePath = Context.toString(arg);
            String sourceCode = readFileToString(new File(filePath));
            Script script = cx.compileString(sourceCode, filePath, 1, null);
            if (script != null) {
                script.exec(cx, WiltonRhinoEnvironment.globalScope());
            }
        }
    }

    static Method getLoadMethod() {
        try {
            return WiltonRhinoScriptLoader.class.getMethod("load", Context.class,
                    Scriptable.class, Object[].class, Function.class);
        } catch (NoSuchMethodException e) {
            throw new RuntimeException(e);
        }
    }

    private static String readFileToString(File file) throws IOException {
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

    private static class StringBuilderWriter extends Writer {
        private final StringBuilder builder;

        /**
         * Construct a new {@link StringBuilder} instance with default capacity.
         */
        public StringBuilderWriter() {
            this.builder = new StringBuilder();
        }

        /**
         * Append a single character to this Writer.
         *
         * @param value The character to append
         * @return This writer instance
         */
        @Override
        public Writer append(char value) {
            builder.append(value);
            return this;
        }

        /**
         * Append a character sequence to this Writer.
         *
         * @param value The character to append
         * @return This writer instance
         */
        @Override
        public Writer append(CharSequence value) {
            builder.append(value);
            return this;
        }

        /**
         * Append a portion of a character sequence to the {@link StringBuilder}.
         *
         * @param value The character to append
         * @param start The index of the first character
         * @param end The index of the last character + 1
         * @return This writer instance
         */
        @Override
        public Writer append(CharSequence value, int start, int end) {
            builder.append(value, start, end);
            return this;
        }

        /**
         * Closing this writer has no effect.
         */
        @Override
        public void close() {
        }

        /**
         * Flushing this writer has no effect.
         */
        @Override
        public void flush() {
        }

        /**
         * Write a String to the {@link StringBuilder}.
         *
         * @param value The value to write
         */
        @Override
        public void write(String value) {
            if (value != null) {
                builder.append(value);
            }
        }

        /**
         * Write a portion of a character array to the {@link StringBuilder}.
         *
         * @param value The value to write
         * @param offset The index of the first character
         * @param length The number of characters to write
         */
        @Override
        public void write(char[] value, int offset, int length) {
            if (value != null) {
                builder.append(value, offset, length);
            }
        }

        /**
         * Returns {@link StringBuilder#toString()}.
         *
         * @return The contents of the String builder.
         */
        @Override
        public String toString() {
            return builder.toString();
        }
    }
}
