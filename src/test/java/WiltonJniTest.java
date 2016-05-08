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

interface WiltonJniTestGateway {
    void gatewayCallback(long requestHandle);
}

class WiltonJniTest {

    static {
        System.loadLibrary("wilton_c");
        System.loadLibrary("wilton_jni");
    }

    private static class TestGateway implements WiltonJniTestGateway {

        @Override
        public void gatewayCallback(long requestHandle) {
            sendResponse(requestHandle, "Hello Java!\n");
        }
    }

    public static void main(String[] args) throws Exception {
        TestGateway gateway = new TestGateway();
        long handle = createServer(gateway, "{\"tcpPort\": 8080}");
        System.console().readLine();
        stopServer(handle);
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
