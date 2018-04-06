/*
 * Copyright 2018, alex at staticlibs.net
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

package net.wiltontoolkit.support.rhino;

import net.wiltontoolkit.WiltonException;
import org.mozilla.javascript.Context;
import org.mozilla.javascript.FunctionObject;
import org.mozilla.javascript.Scriptable;
import org.mozilla.javascript.ScriptableObject;

import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

/**
 * User: alexkasko
 * Date: 4/6/18
 */
public class WiltonRhinoRunner {

    public static void run(String code) throws Exception {
        run(code, "WiltonRhinoRunner");
    }

    public static void run(String code, String path) throws Exception {
        WiltonRhinoEnvironment.initialize(null);
        Context cx = Context.enter();
        ScriptableObject scope = cx.initStandardObjects();
        cx.evaluateString(scope, code, path, 1, null);
        Context.exit();
    }

    public static void main(String[] args) throws Exception {
        if (1 != args.length) {
            System.err.println("ERROR: Path to script not specified");
            return;
        }
        Path path = Paths.get(args[0]);
        if (!Files.exists(path)) {
            System.err.println("ERROR: Specified script not found: [" + path.toString() + "]");
            return;
        }

        String code = new String(Files.readAllBytes(path), StandardCharsets.UTF_8);
        run(code, path.toString());

        System.exit(0);
    }
}
