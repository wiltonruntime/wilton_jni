package net.wiltonwebtoolkit;

class WiltonJniTest {

    static {
        System.loadLibrary("wilton_jni");
    }

    private static native long createServer(Object gateway, String conf);

    private static native void stopServer(long serverHandle);

    private static native String getRequestMetadata(long requestHandle);

    private static native String getRequestData(long requestHandle);

    private static native void setResponseMetadata(long requestHandle, String conf);

    private static native void sendResponse(long requestHandle, String data);

//    TODO
//    private static native void sendResponseChunked(long requestHandle, Object readable);
}
