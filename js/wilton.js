/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

// version 0.5.5

if ("undefined" === typeof(Packages)) {
    console.log("Error: wilton.js requires Nashorn or Rhino JVM environment");
}

// allow to be used without require() as a global 'wilton' object

if ("undefined" === typeof(define) && "undefined" === typeof(wilton)) {
    wilton = null;
    
    define = function(declaration) {
        if (null ===  wilton) {
            wilton = declaration();
        }
    };
}


// definition

define(function() {
    
    // internal utils

    function defaultObject(options) {
        var opts = {};
        if ("object" === typeof(options) && null !== options) {
            opts = options;
        }
        return opts;
    }
    
    function defaultString(str, value) {
        if ("string" === typeof (str)) {
            return str;
        } else {
            if ("undefined" !== typeof(value)) {
                return value;
            } else {
                return String(str);
            }
        }
    }
    
    function defaultJson(data) {
        var dt = "";
        if ("undefined" !== typeof(data) && null !== data) {
            if ("string" === typeof(data)) {
                dt = data;
            } else {
                dt = JSON.stringify(data);
            }
        }
        return dt;
    }
    
    function callOrThrow(onFailure, e, res) {        
        if ("function" === typeof(onFailure)) {
            onFailure(e);
            if ("undefined" !== typeof(res)) {
                return res;
            }
        } else {
            throw e;
        }
    }
    
    function callOrIgnore(onSuccess, params) {
        if ("function" === typeof(onSuccess)) {
            if ("undefined" !== typeof(params)) {
                onSuccess(params);
            } else {
                onSuccess();
            }
        }
    }
    
    function checkAttrType(obj, attr, type) {
        // todo: obj check
        var actual = typeof (obj[attr]);
        if (type !== actual) {
            throw new Error("Invalid attribute specified, name: [" + attr + "]," + 
                    " required type: [" + type + "], actual type: [" + actual + "]");
        }
    }

    // Logger

    var Logger = function(name) {
        this.jni = Packages.net.wiltonwebtoolkit.WiltonJni;
        this.name = defaultString(name, "wilton");
    };
    
    Logger.initialize = function(config) {
        var opts = defaultObject(config);
        var onSuccess = opts.onSuccess;
        var onFailure = opts.onFailure;
        delete opts.onSuccess;
        delete opts.onFailure;
        try {
            var jni = Packages.net.wiltonwebtoolkit.WiltonJni;
            jni.wiltoncall("logger_initialize", JSON.stringify(opts));
            callOrIgnore(onSuccess);
        } catch (e) {
            callOrThrow(onFailure, e);
        }
    };
    
    Logger.shutdown = function(options) {
        var opts = defaultObject(options);
        try {
            var jni = Packages.net.wiltonwebtoolkit.WiltonJni;
            jni.wiltoncall("logger_shutdown");
            callOrIgnore(opts.onSuccess);
        } catch (e) {
            callOrThrow(opts.onFailure, e);
        }
    };

    Logger.prototype = {
        append: function(level, message) {
            try {
                var msg = "";
                if ("undefined" !== typeof(message) && null !== message) {
                    if ("string" === typeof(message)) {
                        msg = message;
                    } else if (message instanceof Error) {
                        msg = message.toString() + "\n" + message.stack;
                    } else {
                        try {
                            msg = JSON.stringify(message);
                        } catch (e) {
                            msg = String(message);
                        }
                    }
                }
                var data = JSON.stringify({
                    level: level,
                    logger: this.name,
                    message: msg
                });
                this.jni.wiltoncall("logger_log", data);
            } catch (e) {
                print("===LOGGER ERROR:");
                print(e.toString() + "\n" + e.stack);
                print("===LOGGER ERROR END:");
            }
        },
        
        log: function(message) {
            this.append("DEBUG", message);
        },
        
        debug: function(message) {
            this.append("DEBUG", message);
        },
        
        info: function(message) {
            this.append("INFO", message);
        },
        
        warn: function(message) {
            this.append("WARN", message);
        },
        
        error: function(message) {
            this.append("ERROR", message);
        }
    };
    
    
    // Response

    var Response = function(server, jni, handle) {
        this.server = server;
        this.jni = jni;
        this.handle = handle;
    };

    Response.prototype = {
        send: function(data, options) {
            var opts = defaultObject(options);
            try {
                this._setMeta(opts);
                var dt = defaultJson(data);
                this.jni.wiltoncall("request_send_response", JSON.stringify({
                    requestHandle: this.handle,
                    data: dt
                }));
                callOrIgnore(opts.onSuccess);
            } catch (e) {
                callOrThrow(opts.onFailure, e);
            }
        },
        
        sendTempFile: function(filePath, options) {
            var opts = defaultObject(options);
            try {
                this._setMeta(opts);
                this.jni.wiltoncall("request_send_temp_file", JSON.stringify({
                    requestHandle: this.handle,
                    filePath: filePath
                }));
                callOrIgnore(opts.onSuccess);
            } catch (e) {
                callOrThrow(opts.onFailure, e);
            }
        },
        
        sendMustache: function(filePath, values, options) {
            var opts = defaultObject(options);
            try {
                this._setMeta(opts);
                var vals = defaultObject(values);
                this.jni.wiltoncall("request_send_mustache", JSON.stringify({
                    requestHandle: this.handle,
                    mustacheFilePath: this.server.mustacheTemplatesRootDir + filePath,
                    values: vals
                }));
                callOrIgnore(opts.onSuccess);
            } catch (e) {
                callOrThrow(opts.onFailure, e);
            }
        },
        
        _setMeta: function(opts) {
            if ("object" === typeof (opts.meta) && null !== opts.meta) {
                this.jni.wiltoncall("request_set_response_metadata", JSON.stringify({
                    requestHandle: this.handle,
                    metadata: opts.meta
                }));
            }
        }
    };


    // Server

    var Server = function(config) {
        var opts = defaultObject(config);
        
        var _prepateViews = function(views) {
            if ("object" !== typeof(views)) {
                throw new Error("Invalid 'views' property");
            }
            if (views instanceof Array) {
                var res = {};
                for (var i = 0; i < views.length; i++) {
                    if ("object" !== typeof(views[i])) {
                        throw new Error("Invalid 'views' array, index: [" + i + "]");
                    }
                    for (var path in views[i]) {
                        var cb = views[i];
                        if (cb.hasOwnProperty(path)) {
                            if (res.hasOwnProperty(path)) {
                                throw new Error("Invalid 'views', duplicate path: [" + path + "]");
                            }
                            res[path] = cb[path];
                        }
                    }
                }
                return res;
            } else {
                return views;
            }
        };
        
        var onSuccess = opts.onSuccess;
        var onFailure = opts.onFailure;
        delete opts.onSuccess;
        delete opts.onFailure;
        try {                        
            this.jni = Packages.net.wiltonwebtoolkit.WiltonJni;
            this.logger = new Logger("wilton.server");
            this.gateway = opts.gateway;
            this.views = _prepateViews(opts.views);
            this.mustacheTemplatesRootDir = "";
            if ("undefined" !== typeof(opts.mustache) && 
                    "string" === typeof(opts.mustache.templatesRootDir)) {
                this.mustacheTemplatesRootDir = opts.mustache.templatesRootDir;
                delete opts.mustache.templatesRootDir;
            }
            var self = this;
            var gatewayPass = new Packages.net.wiltonwebtoolkit.WiltonGateway({
                gatewayCallback: function(requestHandle) {
                    self._gatewaycb(requestHandle);
                }
            });
            delete opts.views;
            var data = JSON.stringify(opts);
            var handleJson = this.jni.wiltoncall("server_create", data, gatewayPass);
            var handleObj = JSON.parse(handleJson);
            this.handle = handleObj.serverHandle;
            callOrIgnore(onSuccess);
        } catch (e) {
            callOrThrow(onFailure, e);
        }
    };

    Server.prototype = {
        _gatewaycb: function(requestHandle) {
            try {
                var json = this.jni.wiltoncall("request_get_metadata", JSON.stringify({
                    requestHandle: requestHandle
                }));
                var req = JSON.parse(json);
                var cb = null;
                if ("function" === typeof(this.gateway)) {
                    cb = gateway;
                } else {
                    cb = this.views[req.pathname];
                    if ("undefined" === typeof(cb)) {
                        this.jni.wiltoncall("request_set_response_metadata", JSON.stringify({
                            requestHandle: requestHandle,
                            metadata: {
                                statusCode: 404,
                                statusMessage: "Not Found"
                            }
                        }));
                        this.jni.wiltoncall("request_send_response", JSON.stringify({
                            requestHandle: requestHandle,
                            data: "404: Not Found: [" + req.pathname + "]"
                        }));
                        return;
                    }
                }
                req.data = "";
                if ("POST" === req.method || "PUT" === req.method) {
                    var bdata = this.jni.wiltoncall("request_get_data", JSON.stringify({
                        requestHandle: requestHandle
                    }));
                    req.data = "" + bdata;
                }
                var resp = new Response(this, this.jni, requestHandle);
                cb(req, resp);
            } catch (e) {
                this.logger.error(e);
                this.jni.wiltoncall("request_set_response_metadata", JSON.stringify({
                    requestHandle: requestHandle,
                    metadata: {
                        statusCode: 500,
                        statusMessage: "Server Error"
                    }
                }));
                this.jni.wiltoncall("request_send_response", JSON.stringify({
                    requestHandle: requestHandle,
                    data: "500: Server Error"
                }));
            }
        },
        
        stop: function(options) {
            var opts = defaultObject(options);
            try {
                this.jni.wiltoncall("server_stop", JSON.stringify({
                    serverHandle: this.handle
                }));
                callOrIgnore(opts.onSuccess);
            } catch (e) {
                callOrThrow(opts.onFailure, e);
            }
        }
    };
    
    
    // Mustache

    var Mustache = function() {
        this.jni = Packages.net.wiltonwebtoolkit.WiltonJni;
    };
    
    Mustache.prototype = {
        render: function(template, values, options) {
            var opts = defaultObject(options);
            try {
                var tp = defaultString(template);
                var vals = defaultObject(values);
                var data = JSON.stringify({
                    template: tp,
                    values: vals
                });
                var res = this.jni.wiltoncall("mustache_render", data);
                var resstr = String(res);
                callOrIgnore(opts.onSuccess, resstr);
                return resstr;
            } catch (e) {
                callOrThrow(opts.onFailure, e, "");
            }
        },
        
        renderFile: function(templateFile, values, options) {
            var opts = defaultObject(options);
            try {
                var tpf = defaultString(templateFile);
                var vals = defaultObject(values);
                var data = JSON.stringify({
                    file: tpf,
                    values: vals
                });
                var res = this.jni.wiltoncall("mustache_render_file", data);
                var resstr = String(res);
                callOrIgnore(opts.onSuccess, resstr);
                return resstr;
            } catch (e) {
                callOrThrow(opts.onFailure, e, "");
            }
        }
    };
    
    
    // Database
    
    var DBConnection = function(config) {
        var opts = defaultObject(config);
        try {
            this.jni = Packages.net.wiltonwebtoolkit.WiltonJni;
            this.url = opts.url;
            var handleJson = this.jni.wiltoncall("db_connection_open", this.url);
            var handleParsed = JSON.parse(handleJson);
            this.handle = handleParsed.connectionHandle;
            callOrIgnore(opts.onSuccess);
        } catch (e) {
            callOrThrow(opts.onFailure, e);
        }
    };
    
    DBConnection.prototype = {
        execute: function(sql, params, options) {
            var opts = defaultObject(options);
            try {
                var sqlstr = defaultString(sql);
                var pars = defaultObject(params);
                this.jni.wiltoncall("db_connection_execute", JSON.stringify({
                    connectionHandle: this.handle,
                    sql: sqlstr,
                    params: pars
                }));
                callOrIgnore(opts.onSuccess);
            } catch (e) {
                callOrThrow(opts.onFailure, e);
            }
        },
        
        queryList: function(sql, params, options) {
            var opts = defaultObject(options);
            try {
                var sqlstr = defaultString(sql);
                var pars = defaultObject(params);
                var json = this.jni.wiltoncall("db_connection_query", JSON.stringify({
                    connectionHandle: this.handle,
                    sql: sqlstr,
                    params: pars
                }));
                var res = JSON.parse(json);
                callOrIgnore(opts.onSuccess, res);
                return res;
            } catch (e) {
                callOrThrow(opts.onFailure, e, []);
            }
        },
        
        query: function(sql, params, options) {
            var opts = defaultObject(options);
            var list = this.queryList(sql, params, {
                onFailure: opts.onFailure
            });
            if (list instanceof Array) {
                var res = null;
                if (list.length > 1) {
                    return list;
                } else if (1 === list.length) {
                    res = list[0];
                }
                callOrIgnore(opts.onSuccess, res);
                return res;
            }
            // else error happened
            return {};
        },
        
        doInTransaction: function(callback, options) {
            var opts = defaultObject(options);
            try {
                var tranJson = this.jni.wiltoncall("db_transaction_start", JSON.stringify({
                    connectionHandle: this.handle
                }));
                var tran = JSON.parse(tranJson);
                try {
                    callback();
                    this.jni.wiltoncall("db_transaction_commit", JSON.stringify({
                        transactionHandle: tran.transactionHandle
                    }));
                } catch (e) {
                    this.jni.wiltoncall("db_transaction_rollback", JSON.stringify({
                        transactionHandle: tran.transactionHandle
                    }));
                    callOrThrow(opts.onFailure, e);
                }
                callOrIgnore(opts.onSuccess);
            } catch (e) {
                callOrThrow(opts.onFailure, e);
            }
        },
        
        close: function(options) {
            var opts = defaultObject(options);
            try {
                this.jni.wiltoncall("db_connection_close", JSON.stringify({
                    connectionHandle: this.handle
                }));
                callOrIgnore(opts.onSuccess);
            } catch (e) {
                callOrThrow(opts.onFailure, e);
            }
        }
    };
    
    
    // HttpClient
    
    var HttpClient = function(config) {
        var opts = defaultObject(config);
        var onSuccess = opts.onSuccess;
        var onFailure = opts.onFailure;
        delete opts.onSuccess;
        delete opts.onFailure;
        try {
            this.jni = Packages.net.wiltonwebtoolkit.WiltonJni;
            var data = JSON.stringify(opts);
            var json = this.jni.wiltoncall("httpclient_create", data);
            var out = JSON.parse(json);
            this.handle = out.httpclientHandle;
            callOrIgnore(onSuccess);
        } catch (e) {
            callOrThrow(onFailure, e);
        }
    };
    
    HttpClient.prototype = {
        execute: function(url, options) {
            var opts = defaultObject(options);
            try {
                var urlstr = defaultString(url);
                var dt = defaultJson(opts.data);
                var meta = defaultObject(opts.meta);
                var resp_json = this.jni.wiltoncall("httpclient_execute", JSON.stringify({
                    httpclientHandle: this.handle,
                    url: urlstr,
                    data: dt,
                    metadata: meta
                }));
                var resp = JSON.parse(resp_json);
                callOrIgnore(opts.onSuccess, resp);
                return resp;
            } catch (e) {
                callOrThrow(opts.onFailure, e);
            }
        },
        
        sendTempFile: function(url, options) {
            var opts = defaultObject(options);
            try {
                var urlstr = defaultString(url);
                var fp = defaultJson(opts.filePath);
                var meta = defaultObject(opts.meta);
                var resp_json = this.jni.wiltoncall("httpclient_send_temp_file", JSON.stringify({
                    httpclientHandle: this.handle,
                    url: urlstr,
                    filePath: fp,
                    metadata: meta
                }));
                var resp = JSON.parse(resp_json);
                callOrIgnore(opts.onSuccess, resp);
                return resp;
            } catch (e) {
                callOrThrow(opts.onFailure, e, {});
            }
        },
        
        close: function(options) {
            var opts = defaultObject(options);
            try {
                this.jni.wiltoncall("httpclient_close", JSON.stringify({
                    httpclientHandle: this.handle
                }));
                callOrIgnore(opts.onSuccess);
            } catch (e) {
                callOrThrow(opts.onFailure, e);
            }
        }
    };
    
    
    // Cron
    
    var CronTask = function(config) {
        var opts = defaultObject(config);
        checkAttrType(opts, "callback", "function");
        var onSuccess = opts.onSuccess;
        var onFailure = opts.onFailure;
        delete opts.onSuccess;
        delete opts.onFailure;
        try {
            this.jni = Packages.net.wiltonwebtoolkit.WiltonJni;
            var cb = opts.callback;
            delete opts.callback;
            var runnable = new Packages.java.lang.Runnable({
                run: function() {
                    cb();
                }
            });
            var data = JSON.stringify(opts);
            var handleJson = this.jni.wiltoncall("cron_start", data, runnable);
            var handleObj = JSON.parse(handleJson);
            this.handle = handleObj.cronHandle;
            callOrIgnore(onSuccess);
        } catch (e) {
            callOrThrow(onFailure, e);
        }
    };
    
    CronTask.prototype = {
        stop: function(options) {
            var opts = defaultObject(options);
            try {
                this.jni.wiltoncall("cron_stop", JSON.stringify({
                    cronHandle: this.handle
                }));
                callOrIgnore(opts.onSuccess);
            } catch (e) {
                callOrThrow(opts.onFailure, e);
            }
        }
    };
    
    
    // Mutex
    
    var Mutex = function(options) {
        var opts = defaultObject(options);
        try {
            this.jni = Packages.net.wiltonwebtoolkit.WiltonJni;
            var handleJson = this.jni.wiltoncall("mutex_create");
            var handleObj = JSON.parse(handleJson);
            this.handle = handleObj.mutexHandle;
            callOrIgnore(opts.onSuccess);
        } catch (e) {
            callOrThrow(opts.onFailure, e);
        }
    };
    
    Mutex.prototype = {
        synchronized: function(options) {
            var opts = defaultObject(options);
            checkAttrType(opts, "callback", "function");
            try {
                var data = JSON.stringify({
                    mutexHandle: this.handle
                });
                var res = {};
                try {
                    this.jni.wiltoncall("mutex_lock", data);
                    res = opts.callback();
                } finally {
                    this.jni.wiltoncall("mutex_unlock", data);
                }                
                callOrIgnore(opts.onSuccess, res);
                return res;
            } catch (e) {
                callOrThrow(opts.onFailure, e, {});
            }
        },
        
        lock: function(options) {
            this._voidcall("lock", options);
        },
        
        unlock: function(options) {
            this._voidcall("unlock", options);
        },
        
        destroy: function(options) {
            this._voidcall("destroy", options);
        },
        
        _voidcall: function(name, options) {
            var opts = defaultObject(options);
            try {
                this.jni.wiltoncall("mutex_" + name, JSON.stringify({
                    mutexHandle: this.handle
                }));
                callOrIgnore(opts.onSuccess);
            } catch (e) {
                callOrThrow(opts.onFailure, e);
            }
        }
    };
    
    
    // Misc
    
    var Misc = function() {
        throw new Error("'Misc' object cannot be instantiated, use its functions directly instead.");
    };
        
    Misc.threadSleepMillis = function(millis, options) {
        var opts = defaultObject(options);
        try {            
            var jni = Packages.net.wiltonwebtoolkit.WiltonJni;
            jni.wiltoncall("thread_sleep_millis", JSON.stringify({
                millis: millis
            }));
            callOrIgnore(opts.onSuccess);
        } catch (e) {
            callOrThrow(opts.onFailure, e);
        }
    };
    
    Misc.tcpWaitForConnection = function(options) {
        var opts = defaultObject(options);
        var onSuccess = opts.onSuccess;
        var onFailure = opts.onFailure;
        delete opts.onSuccess;
        delete opts.onFailure;
        try {
            var jni = Packages.net.wiltonwebtoolkit.WiltonJni;
            jni.wiltoncall("tcp_wait_for_connection", JSON.stringify(opts));
            callOrIgnore(onSuccess);
        } catch (e) {
            callOrThrow(onFailure, e);
        }
    };
    
    // export
    
    return {
        DBConnection: DBConnection,
        HttpClient: HttpClient,
        Logger: Logger,
        Mustache: Mustache,
        Server: Server,
        CronTask: CronTask,
        Mutex: Mutex,
        Misc: Misc
    };
    
});
