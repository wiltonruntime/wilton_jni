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

import net.wiltontoolkit.support.rhino.WiltonRhinoEnvironment;
import org.junit.BeforeClass;
import org.junit.Test;
import org.mozilla.javascript.*;
import utils.TestGateway;

import static org.junit.Assert.assertEquals;
import static utils.TestUtils.*;

/**
 * User: alexkasko
 * Date: 5/15/16
 */
public class WiltonRhinoTest {

    @BeforeClass
    public static void init() throws Exception {
        // init, no logging by default, enable it when needed
        initWiltonOnce(new TestGateway(), LOGGING_DISABLE);
    }

    @Test
    // https://github.com/mozilla/rhino/issues/153
    public void testStacktrace() throws Exception {
        // force static init
        WiltonRhinoEnvironment.initialize(null);
        Context cx = Context.enter();
        Scriptable scope = cx.initStandardObjects();
        Object[] holder = new Object[1];
        ScriptableObject.putProperty(scope, "holder", Context.javaToJS(holder, scope));
        cx.evaluateString(scope, "try {throw new Error() } catch (e) {holder[0] = e.stack}", "some_file.js", 42, null);
        assertEquals("\tat some_file.js:42" + System.lineSeparator(), holder[0]);
        Context.exit();
    }
}
