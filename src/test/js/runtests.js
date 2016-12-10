/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

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
            "tests/LoggerTest",
            "tests/CronTaskTest",
            "tests/DBConnectionTest",
            "tests/HttpClientTest",
            "tests/MutexTest",
            "tests/ServerTest",
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
