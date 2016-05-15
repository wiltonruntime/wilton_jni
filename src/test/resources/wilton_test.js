
var out = Packages.java.lang.System.out;
var httpGet = Packages.utils.TestUtils.httpGet;
var httpGetHeader = Packages.utils.TestUtils.httpGetHeader;
var httpPost = Packages.utils.TestUtils.httpPost;
var assertEquals = Packages.org.junit.Assert.assertEquals;

var errorCb = function (e) {
    throw e;
};

var server = new wilton.Server({
    tcpPort: 8080,
    logging: {
        appenders: [{
            appenderType: "CONSOLE",
            thresholdLevel: "WARN" // lower me for debugging
        }],
        loggers: [{
            name: "staticlib.httpserver",
            level: "INFO"
        }, {
            name: "wilton",
            level: "DEBUG"
        }]
    }, callbacks: {
        "/hi": function(req, resp) {
            resp.send("Hi from wilton_test!");
        },
        "/path/to/json": function(req, resp) {
            resp.send({
                foo: 1,
                bar: "baz"
            });
        },
        "/error/reporting": function(req, resp) {
            resp.send("", {"foo": "bar"}, null, function () {
                resp.send("Error triggered");
            });
        },
        "/req/header": function(req, resp) {
            resp.send(req.headers["Host"]);
        },
        "/resp/xfoo/header": function (req, resp) {
            resp.send("header set", {
                headers: {
                    "X-Foo": "foo"
                }
            }, null, errorCb);
        },
        "/postmirror": function (req, resp) {
            resp.send(req.data, null, null, errorCb);
        }
    }
}, null, errorCb);

var prefix = "http://127.0.0.1:8080";
assertEquals("404: Not Found: [/foo]", httpGet(prefix + "/foo"));
assertEquals("Hi from wilton_test!", httpGet(prefix + "/hi"));
assertEquals("{\"foo\":1,\"bar\":\"baz\"}", httpGet(prefix + "/path/to/json"));
assertEquals("Error triggered", httpGet(prefix + "/error/reporting"));
assertEquals("127.0.0.1:8080", httpGet(prefix + "/req/header"));
assertEquals("header set", httpGet(prefix + "/resp/xfoo/header"));
assertEquals("foo", httpGetHeader(prefix + "/resp/xfoo/header", "X-Foo"));
assertEquals("foobar", httpPost(prefix + "/postmirror", "foobar"));

server.stop(null, errorCb);
