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

import net.wiltonwebtoolkit.HttpGateway;
import static net.wiltonwebtoolkit.HttpServerJni.createServer;
import static net.wiltonwebtoolkit.HttpServerJni.sendResponse;
import static net.wiltonwebtoolkit.HttpServerJni.stopServer;
import org.junit.Test;

class WiltonJniTest {

    private static class TestGateway implements HttpGateway {
        @Override
        public void gatewayCallback(long requestHandle) {
            sendResponse(requestHandle, "Hello Java!\n");
        }
    }

    @Test
    public void test() {
        TestGateway gateway = new TestGateway();
        long handle = createServer(gateway, "{\"tcpPort\": 8080}");
//        System.console().readLine();
        stopServer(handle);
    }

}
