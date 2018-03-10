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

import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableMap;
import net.wiltontoolkit.support.nashorn.WiltonNashornEnvironment;
import org.junit.BeforeClass;
import org.junit.Test;
import utils.TestGateway;
import utils.TestUtils;

import static net.wiltontoolkit.WiltonJni.LOGGING_DISABLE;
import static utils.TestUtils.GSON;
import static utils.TestUtils.getJsDir;
import static utils.TestUtils.initWiltonOnce;

/**
 * User: alexkasko
 * Date: 5/15/16
 */
public class WiltonNashornTest {

    @BeforeClass
    public static void init() {
        String wiltonDirPath = getJsDir().getAbsolutePath();
        // init, no logging by default, enable it when needed
        initWiltonOnce(new TestGateway(), LOGGING_DISABLE, wiltonDirPath);
        TestGateway tg = (TestGateway) TestUtils.GATEWAY;
        WiltonNashornEnvironment.initialize(wiltonDirPath);
        tg.setScriptGateway(WiltonNashornEnvironment.gateway());
    }

    @Test
    public void test() throws Exception {
        // wilton test suite
        WiltonNashornEnvironment.gateway().runScript(GSON.toJson(ImmutableMap.builder()
                .put("module", "../js/wilton/test/index")
                .put("func", "main")
                .build()));
        // node modules tests
        WiltonNashornEnvironment.gateway().runScript(GSON.toJson(ImmutableMap.builder()
                .put("module", "test/scripts/runNodeTests")
                .put("func", "")
                .build()));
        // sanity
        WiltonNashornEnvironment.gateway().runScript(GSON.toJson(ImmutableMap.builder()
                .put("module", "test/scripts/runSanityTests")
                .put("func", "")
                .build()));
    }
}
