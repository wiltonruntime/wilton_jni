
var out = Packages.java.lang.System.out;
var httpGet = Packages.utils.TestUtils.httpGet;
var httpGetHeader = Packages.utils.TestUtils.httpGetHeader;
var httpPost = Packages.utils.TestUtils.httpPost;
var assertEquals = Packages.org.junit.Assert.assertEquals;

var errorCb = function (e) {
    throw e;
};

// Server

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

// HttpClient
var http = new wilton.HttpClient();
var resp = http.execute(prefix + "/hi", {
    forceHttp10: true
}, null, errorCb);
assertEquals("Hi from wilton_test!", resp.data);
assertEquals("close", resp.headers.Connection);
var resp = http.execute(prefix + "/postmirror", {
    data: "foobar",
    forceHttp10: true
}, null, errorCb);
assertEquals("foobar", resp.data);
http.close();

server.stop(null, errorCb);

// Mustache

var mustache = new wilton.Mustache();
assertEquals("Hi Chris!\nHi Mark!\nHi Scott!\n", mustache.render("{{#names}}Hi {{name}}!\n{{/names}}", 
    {names: [{name: "Chris"}, {name: "Mark"}, {name: "Scott"}]}, null, errorCb));

// DBConnection

var conn = new wilton.DBConnection({url: "sqlite://target/test.db"}, null, errorCb);
conn.execute("drop table if exists t1", null, null, errorCb);
// insert
conn.execute("create table t1 (foo varchar, bar int)", null, null, errorCb);
conn.execute("insert into t1 values('aaa', 41)", null, null, errorCb);
// named params
conn.execute("insert into t1 values(:foo, :bar)", {
    foo: "bbb",
    bar: 42
}, null, errorCb);
conn.execute("insert into t1 values(?, ?)", ["ccc", 43], null, errorCb);
// select
var rs = conn.query("select foo, bar from t1 where foo = :foo or bar = :bar order by bar", {
    foo: "ccc",
    bar: 42
});
// Junit float compare fail
assertEquals("2", String(rs.length));
assertEquals("bbb", rs[0].foo);
assertEquals("42", String(rs[0].bar));
assertEquals("ccc", rs[1].foo);
assertEquals("43", String(rs[1].bar));
var el = conn.query("select foo, bar from t1 where foo = :foo or bar = :bar order by bar", {
    foo: "bbb",
    bar: 42
});
assertEquals("bbb", el.foo);
assertEquals("42", String(el.bar));

conn.doInTransaction(function() {/* some db actions */}, null, errorCb);

conn.close();
