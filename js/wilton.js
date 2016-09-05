/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

// version 0.2.5

if ("undefined" === typeof (Packages)) {
    console.log("Error: wilton.js requires Nashorn or Rhino JVM environment");
}

// can be used without require() as a global 'wilton' object

if ("undefined" === typeof (define) && "undefined" === typeof (wilton)) {
    wilton = null;
    
    define = function(declaration) {
        if (null ===  wilton) {
            wilton = declaration();
        }
    };
}


// definition

define(function () {

    // Logger

    var Logger = function (name) {
        this.jni = Packages.net.wiltonwebtoolkit.WiltonJni;
        this.name = "string" === typeof (name) ? name : "wilton";
    };

    Logger.prototype = {
        append: function (level, message) {
            try {
                if (message instanceof Error) {
                    message = message.toString() + "\n" + message.stack;
                } else if ("string" !== typeof (message)) {
                    message = String(message);
                    try {
                        message = JSON.stringify(message);
                    } catch (e) {
                        // log as-is
                    }
                }
                var data = JSON.stringify({
                    level: level,
                    logger: this.name,
                    message: message
                });
                this.jni.wiltoncall("logger_log", data);
            } catch (e) {
                print("===LOGGER ERROR:");
                print(e.toString() + "\n" + e.stack);
                print("===LOGGER ERROR END:");
            }
        },
        log: function (message) {
            this.append("DEBUG", message);
        },
        debug: function (message) {
            this.append("DEBUG", message);
        },        
        info: function (message) {
            this.append("INFO", message);
        },
        warn: function (message) {
            this.append("WARN", message);
        },
        error: function (message) {
            this.append("ERROR", message);
        }
    };
    
    
    // Response

    var Response = function (server, jni, handle) {
        this.server = server;
        this.jni = jni;
        this.handle = handle;
    };

    Response.prototype = {
        send: function (data, options) {
            try {
                if ("object" === typeof (options) && null !== options && "object" === typeof (options.meta)) {
                    this.jni.wiltoncall("request_set_response_metadata", JSON.stringify({
                        requestHandle: this.handle,
                        metadata: options.meta
                    }));
                }
                if ("undefined" === typeof (data) || null === data) {
                    data = "";
                } else if ("string" !== typeof (data)) {
                    data = JSON.stringify(data);
                }
                this.jni.wiltoncall("request_send_response", JSON.stringify({
                    requestHandle: this.handle,
                    data: data
                }));
                if ("object" === typeof (options) && null !== options && "function" === typeof (options.onSuccess)) {
                    options.onSuccess();
                }
            } catch (e) {
                if ("object" === typeof (options) && null !== options && "function" === typeof (options.onError)) {
                    options.onError(e);
                }
            }
        },
        sendTempFile: function (filePath, options) {
            try {
                if ("object" === typeof (options) && null !== options && "object" === typeof (options.meta)) {
                    this.jni.wiltoncall("request_set_response_metadata", JSON.stringify({
                        requestHandle: this.handle,
                        metadata: options.meta
                    }));
                }
                this.jni.wiltoncall("request_send_temp_file", JSON.stringify({
                    requestHandle: this.handle,
                    filePath: filePath
                }));
                if ("object" === typeof (options) && null !== options && "function" === typeof (options.onSuccess)) {
                    options.onSuccess();
                }
            } catch (e) {
                if ("object" === typeof (options) && null !== options && "function" === typeof (options.onError)) {
                    options.onError(e);
                }
            }
        },
        sendMustache: function (filePath, data, options) {
            try {
                if ("object" === typeof (options) && null !== options && "object" === typeof (options.meta)) {
                    this.jni.wiltoncall("request_set_response_metadata", JSON.stringify({
                        requestHandle: this.handle,
                        metadata: options.meta
                    }));
                }
                if ("undefined" === typeof (data) || null === data) {
                    data = {};
                } 
                this.jni.wiltoncall("request_send_mustache", JSON.stringify({
                    requestHandle: this.handle,
                    mustacheFilePath: filePath,
                    values: data
                }));
                if ("object" === typeof (options) && null !== options && "function" === typeof (options.onSuccess)) {
                    options.onSuccess();
                }
            } catch (e) {
                if ("object" === typeof (options) && null !== options && "function" === typeof (options.onError)) {
                    options.onError(e);
                }
            }
        }        
    };


    // Server

    var Server = function (conf) {
        try {
            this.jni = Packages.net.wiltonwebtoolkit.WiltonJni;
            this.logger = new Logger("wilton.server");
            this.gateway = conf.gateway;
            this.callbacks = conf.callbacks;
            var onSuccess = conf.onSuccess;
            var onError = conf.onError;
            delete conf.onSuccess;
            delete conf.onError;
            var self = this;
            var gatewayPass = new Packages.net.wiltonwebtoolkit.WiltonGateway({
                gatewayCallback: function (requestHandle) {
                    self.gatewayCallback(requestHandle);
                }
            });
            delete conf.callbacks;
            var data = JSON.stringify(conf);
            var handleJson = this.jni.wiltoncall("server_create", data, gatewayPass);
            var handleObj = JSON.parse(handleJson);
            this.handle = handleObj.serverHandle;
            if ("function" === typeof (onSuccess)) {
                onSuccess(this);
            }
        } catch (e) {
            if ("function" === typeof (onError)) {
                onError(e);
            }
        }
    };

    Server.prototype = {
        gatewayCallback: function (requestHandle) {
            try {
                var json = this.jni.wiltoncall("request_get_metadata", JSON.stringify({
                    requestHandle: requestHandle
                }));
                var req = JSON.parse(json);
                var cb = null;
                if ("function" === typeof (this.gateway)) {
                    cb = gateway;
                } else {
                    cb = this.callbacks[req.pathname];
                    if ("undefined" === typeof (cb)) {
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
                var resp = new Response(this.server, this.jni, requestHandle);
                cb(req, resp);
            } catch (e) {
                this.server.logger.error(e);
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
        stop: function (options) {
            try {
                this.jni.wiltoncall("server_stop", JSON.stringify({
                    serverHandle: this.handle
                }));
                if ("object" === typeof (options) && null !== options && "function" === typeof (options.onSuccess)) {
                    options.onSuccess();
                }
            } catch (e) {
                if ("object" === typeof (options) && null !== options && "function" === typeof (options.onError)) {
                    options.onError(e);
                }
            }
        }
    };
    
    // Mustache

    var Mustache = function () {
        this.jni = Packages.net.wiltonwebtoolkit.WiltonJni;
    };
    
    Mustache.prototype = {
        // todo: revisit API
        render: function(template, values, options) {
            try {
                if ("string" !== typeof (template)) {
                    template = String(template);
                }
                if ("undefined" === typeof (values) || null === values) {
                    values = {};
                }
                var data = JSON.stringify({
                    template: template,
                    values: values
                });
                var res = this.jni.wiltoncall("mustache_render", data);
                if ("object" === typeof (options) && null !== options && "function" === typeof (options.onSuccess)) {
                    options.onSuccess();
                }
                return res;
            } catch (e) {
                if ("object" === typeof (options) && null !== options && "function" === typeof (options.onError)) {
                    options.onError(e);
                }
            }
        }
    };
    
    // Database
    
    var DBConnection = function (conf) {
        try {
            this.jni = Packages.net.wiltonwebtoolkit.WiltonJni;
            this.url = conf.url;
            var handleJson = this.jni.wiltoncall("db_connection_open", this.url);
            var handleParsed = JSON.parse(handleJson);
            this.handle = handleParsed.connectionHandle;
            if ("function" === typeof (conf.onSuccess)) {
                conf.onSuccess(this);
            }
        } catch (e) {
            if ("function" === typeof (conf.onError)) {
                conf.onError(e);
            }
        }
    };
    
    DBConnection.prototype = {
        execute: function(sql, params, options) {
            try {
                if ("string" !== typeof (sql)) {
                    sql = String(sql);
                }
                if ("undefined" === typeof (params) || null === params) {
                    params = {};
                }
                this.jni.wiltoncall("db_connection_execute", JSON.stringify({
                    connectionHandle: this.handle,
                    sql: sql,
                    params: params
                }));
                if ("object" === typeof (options) && null !== options && "function" === typeof (options.onSuccess)) {
                    options.onSuccess();
                }
            } catch (e) {
                if ("object" === typeof (options) && null !== options && "function" === typeof (options.onError)) {
                    options.onError(e);
                }
            }
        },
        
        queryList: function(sql, params, options) {
            try {
                if ("string" !== typeof (sql)) {
                    sql = String(sql);
                }
                if ("undefined" === typeof (params) || null === params) {
                    params = "";
                }
                var json = this.jni.wiltoncall("db_connection_query", JSON.stringify({
                    connectionHandle: this.handle,
                    sql: sql,
                    params: params
                }));
                var res = JSON.parse(json);
                if ("object" === typeof (options) && null !== options && "function" === typeof (options.onSuccess)) {
                    options.onSuccess(res);
                }
                return res;
            } catch (e) {
                if ("object" === typeof (options) && null !== options && "function" === typeof (options.onError)) {
                    options.onError(e);
                }
            }
        },
        
        query: function(sql, params, options) {
            var onError = null;
            if ("object" === typeof (options) && null !== options && "function" === typeof (options.onError)) {
                onError = options.onError;
            }
            var list = this.queryList(sql, params, {
                onError: onError
            });
            if (list instanceof Array) {
                var res = null;
                if (list.length > 1) {
                    return list;
                } else if (1 === list.length) {
                    res = list[0];
                }
                if ("object" === typeof (options) && null !== options && "function" === typeof (options.onSuccess)) {
                    options.onSuccess(res);
                }
                return res;
            }
            // else error happened
        },
        
        doInTransaction: function (callback, options) {
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
                    if ("object" === typeof (options) && null !== options && "function" === typeof (options.onError)) {
                        options.onError(e);
                    }
                }
                if ("object" === typeof (options) && null !== options && "function" === typeof (options.onSuccess)) {
                    options.onSuccess();
                }
            } catch (e) {
                if ("object" === typeof (options) && null !== options && "function" === typeof (options.onError)) {
                    options.onError(e);
                }
            }
        },
        
        close: function(options) {
            try {
                this.jni.wiltoncall("db_connection_close", JSON.stringify({
                    connectionHandle: this.handle
                }));
                if ("object" === typeof (options) && null !== options && "function" === typeof (options.onSuccess)) {
                    options.onSuccess();
                }
            } catch (e) {
                if ("object" === typeof (options) && null !== options && "function" === typeof (options.onError)) {
                    options.onError(e);
                }
            }
        }
    };
    
    // HttpClient
    
    var HttpClient = function (conf) {
        try {
            if ("object" !== typeof(conf) || null === conf) {
                conf = {};
            }
            var onSuccess = conf.onSuccess;
            var onError = conf.onError;
            delete conf.onSuccess;
            delete conf.onError;
            this.jni = Packages.net.wiltonwebtoolkit.WiltonJni;
            var data = JSON.stringify(conf);
            var json = this.jni.wiltoncall("httpclient_create", data);
            var out = JSON.parse(json);
            this.handle = out.httpclientHandle;
            if ("function" === typeof (onSuccess)) {
                onSuccess(this);
            }
        } catch (e) {
            if ("function" === typeof (onError)) {
                onError(e);
            }
        }
    };
    
    HttpClient.prototype = {
        execute: function(url, options) {
            try {
                if ("undefined" === typeof (url) || null === url) {
                    url = "";
                } else if ("string" !== typeof (url)) {
                    url = String(url);
                }
                var requestData = "";
                var onSuccess = null;
                var onError = null;
                if ("object" === typeof (options) && null !== options) {
                    onSuccess = options.onSuccess;
                    onError = options.onError;
                    delete options.onSuccess;
                    delete options.onError;
                    if ("undefined" !== typeof (options.data)) {
                        if (null !== options.data) {
                            if ("string" !== typeof (options.data)) {
                                requestData = JSON.stringify(options.data);
                            } else {
                                requestData = options.data;
                            }
                        }
                        delete options.data;
                    }
                }
                var data = JSON.stringify({
                    httpclientHandle: this.handle,
                    url: url,
                    data: requestData,
                    metadata: options
                });
                var resp_json = this.jni.wiltoncall("httpclient_execute", data);
                var resp = JSON.parse(resp_json);
                if ("function" === typeof (onSuccess)) {
                    onSuccess(resp);
                }
                return resp;
            } catch (e) {
                if ("function" === typeof (onError)) {
                    onError(e);
                }
            }
        },
        
        sendTempFile: function(url, filePath, options) {
            try {
                if ("undefined" === typeof (url) || null === url) {
                    url = "";
                } else if ("string" !== typeof (url)) {
                    url = String(url);
                }
                if ("undefined" === typeof (filePath) || null === filePath) {
                    filePath = "";
                } else if ("string" !== typeof (filePath)) {
                    filePath = String(filePath);
                }
                var metadata = {};
                if ("object" === typeof (options) && null !== options && 
                        "object" === typeof (options.meta) && null !== options.meta) {
                    metadata = options.meta;
                }
                var data = JSON.stringify({
                    httpclientHandle: this.handle,
                    url: url,
                    filePath: filePath,
                    metadata: metadata
                });
                var resp_json = this.jni.wiltoncall("httpclient_send_temp_file", data);
                var resp = JSON.parse(resp_json);
                if ("object" === typeof (options) && null !== options && "function" === typeof (options.onSuccess)) {
                    options.onSuccess(resp);
                }
                return resp;
            } catch (e) {
                if ("object" === typeof (options) && null !== options && "function" === typeof (options.onError)) {
                    options.onError(e);
                }
            }
        },
        
        close: function(options) {
            try {
                var data = JSON.stringify({
                    httpclientHandle: this.handle
                });
                this.jni.wiltoncall("httpclient_close", data);
                if ("object" === typeof (options) && null !== options && "function" === typeof (options.onSuccess)) {
                    options.onSuccess();
                }
            } catch (e) {
                if ("object" === typeof (options) && null !== options && "function" === typeof (options.onError)) {
                    options.onError(e);
                }
            }
        }
    };
    
    // export
    
    return {
        DBConnection: DBConnection,
        HttpClient: HttpClient,
        Logger: Logger,
        Mustache: Mustache,
        Server: Server
    };
    
});
