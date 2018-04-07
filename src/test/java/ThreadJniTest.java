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
import org.junit.BeforeClass;
import org.junit.Test;
import utils.TestGateway;

import static wilton.WiltonJni.wiltoncall;
import static org.junit.Assert.assertTrue;
import static utils.TestUtils.*;

/**
 * User: alexkasko
 * Date: 12/10/16
 */
public class ThreadJniTest {

    @BeforeClass
    public static void init() {
        // init, no logging by default, enable it when needed
        initWiltonOnce(new TestGateway(), LOGGING_DISABLE);
    }

    @Test
    public void testRun() throws Exception {
        wiltoncall("thread_run", GSON.toJson(ImmutableMap.builder()
                .put("callbackScript", ImmutableMap.builder()
                        .put("module", "thread/test")
                        .put("func", "testRun")
                        .put("args", ImmutableList.of())
                        .build())
                .build()));
        TestGateway.threadTestLatch.await();
        assertTrue(Thread.currentThread().getId() != TestGateway.threadTestId.get());
    }

    @Test
    public void testSleep() throws Exception {
        long start = System.currentTimeMillis();
        wiltoncall("thread_sleep_millis", GSON.toJson(ImmutableMap.builder()
                .put("millis", 50)
                .build()));
        long stop = System.currentTimeMillis();
        assertTrue((stop - start) > 30);
    }
}
