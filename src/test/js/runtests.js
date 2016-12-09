/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

// todo: removeme
var out = Packages.java.lang.System.out;
var httpGet = Packages.utils.TestUtils.httpGet;
var httpGetHeader = Packages.utils.TestUtils.httpGetHeader;
var httpPost = Packages.utils.TestUtils.httpPost;
var assertEquals = Packages.org.junit.Assert.assertEquals;
var assertTrue = Packages.org.junit.Assert.assertTrue;

(function() {
    "use strict";

    // setup requirejs
    load("src/test/js/requirejs/require.js");
    load("src/test/js/requirejs/rhino.js");
    requirejs.config({
        baseUrl: "src/test/js/modules/"
    });

    try {
        // run tests
        require([
            "tests/CronTaskTest",
            "tests/DBConnectionTest",
            "tests/HttpClientTest",
            "tests/LoggerTest",
            "tests/MutexTest",
            "tests/ServerTest",
            "tests/ThreadTest",
            "tests/miscTest",
            "tests/mustacheTest",
            "tests/nativeLibTest",
            "tests/threadTest",
            "tests/sharedTest",
            "tests/utilsTest"
        ], function() { });
    } catch(e) {
        throw new Error(e.message + "\n" + e.stack);
    }
    
}());
