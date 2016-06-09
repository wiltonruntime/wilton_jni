/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

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
        this.jni = Packages.net.wiltonwebtoolkit.HttpServerJni;
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
                this.jni.appendLog(level, this.name, message);
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
        send: function (data, metadata, onSuccess, onError) {
            try {
                if ("object" === typeof (metadata)) {
                    var json = JSON.stringify(metadata);
                    this.jni.setResponseMetadata(this.handle, json);
                }
                if ("undefined" === typeof (data) || null === data) {
                    data = "";
                } else if ("string" !== typeof (data)) {
                    data = JSON.stringify(data);
                }
                this.jni.sendResponse(this.handle, data);
                if ("function" === typeof (onSuccess)) {
                    onSuccess();
                }
            } catch (e) {
                if ("function" === typeof (onError)) {
                    onError(e);
                }
            }
        },
        sendTempFile: function (filePath, metadata, onSuccess, onError) {
            try {
                if ("object" === typeof (metadata)) {
                    var json = JSON.stringify(metadata);
                    this.jni.setResponseMetadata(this.handle, json);
                }
                this.jni.sendTempFile(this.handle, filePath);
                if ("function" === typeof (onSuccess)) {
                    onSuccess();
                }
            } catch (e) {
                if ("function" === typeof (onError)) {
                    onError(e);
                }
            }
        },
        sendMustache: function (filePath, data, metadata, onSuccess, onError) {
            try {
                if ("object" === typeof (metadata)) {
                    var json = JSON.stringify(metadata);
                    this.jni.setResponseMetadata(this.handle, json);
                }
                if ("undefined" === typeof (data) || null === data) {
                    data = "{}";
                } else if ("string" !== typeof (data)) {
                    data = JSON.stringify(data);
                }
                this.jni.sendMustache(this.handle, filePath, data);
                if ("function" === typeof (onSuccess)) {
                    onSuccess();
                }
            } catch (e) {
                if ("function" === typeof (onError)) {
                    onError(e);
                }
            }
        }        
    };


    // Gateway

    var Gateway = function (conf) {
        this.server = conf.server;
        this.jni = conf.jni;
        this.gateway = conf.gateway;
        this.callbacks = conf.callbacks;
    };

    Gateway.prototype = {
        gatewayCallback: function (requestHandle) {
            try {
                var json = this.jni.getRequestMetadata(requestHandle);
                var req = JSON.parse(json);
                var cb = null;
                if ("function" === typeof (this.gateway)) {
                    cb = gateway;
                } else {
                    cb = this.callbacks[req.pathname];
                    if ("undefined" === typeof (cb)) {
                        var rm = JSON.stringify({
                            statusCode: 404,
                            statusMessage: "Not Found"
                        });
                        this.jni.setResponseMetadata(requestHandle, rm);
                        this.jni.sendResponse(requestHandle, "404: Not Found: [" + req.pathname + "]");
                        return;
                    }
                }
                req.data = "";
                if ("POST" === req.method || "PUT" === req.method) {
                    var bdata = this.jni.getRequestData(requestHandle);
                    req.data = "" + bdata;
                }
                var resp = new Response(this.server, this.jni, requestHandle);
                cb(req, resp);
            } catch (e) {
                this.server.logger.error(e);
                var rm = JSON.stringify({
                    statusCode: 500,
                    statusMessage: "Server Error"
                });
                this.jni.setResponseMetadata(requestHandle, rm);
                this.jni.sendResponse(requestHandle, "500: Server Error");
            }
        }
    };


    // Server

    var Server = function (conf, onSuccess, onError) {
        try {
            this.jni = Packages.net.wiltonwebtoolkit.HttpServerJni;
            this.logger = new Logger("wilton.server");
            this.gateway = new Gateway({
                server: this,
                jni: this.jni,
                gateway: conf.gateway,
                callbacks: conf.callbacks
            });
            var self = this;
            var gatewayPass = new Packages.net.wiltonwebtoolkit.HttpGateway({
                gatewayCallback: function (requestHandle) {
                    self.gateway.gatewayCallback(requestHandle);
                }
            });
            delete conf.callbacks;
            var confJson = JSON.stringify(conf);
            this.handle = this.jni.createServer(gatewayPass, confJson);
            if ("function" === typeof (onSuccess)) {
                onSuccess();
            }
        } catch (e) {
            if ("function" === typeof (onError)) {
                onError(e);
            }
        }
    };

    Server.prototype = {
        stop: function (onSuccess, onError) {
            try {
                this.jni.stopServer(this.handle);
                if ("function" === typeof (onSuccess)) {
                    onSuccess();
                }
            } catch (e) {
                if ("function" === typeof (onError)) {
                    onError(e);
                }
            }
        }
    };
    
    // Mustache

    var Mustache = function () {
        this.jni = Packages.net.wiltonwebtoolkit.HttpServerJni;
    };
    
    Mustache.prototype = {
        render: function(template, values, onSuccess, onError) {
            try {
                if ("string" !== typeof (template)) {
                    template = String(template);
                }
                if ("undefined" === typeof (values) || null === values) {
                    values = "{}";
                } else if ("string" !== typeof (values)) {
                    values = JSON.stringify(values);
                }
                var res = this.jni.renderMustache(template, values);
                if ("function" === typeof (onSuccess)) {
                    onSuccess(res);
                }
                return res;
            } catch (e) {
                if ("function" === typeof (onError)) {
                    onError(e);
                }
            }
        }
    };
    
    // export
    
    return {
        Logger: Logger,
        Mustache: Mustache,
        Server: Server
    };
    
});
