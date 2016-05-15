/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

if ("undefined" === typeof (Packages)) {
    console.log("Error: wilton.js requires Nashorn or Rhino JVM environment");
}

if ("undefined" === typeof (wilton)) {
    wilton = {};
}


// console

wilton.Console = function() {
    this.jni = Packages.net.wiltonwebtoolkit.HttpServerJni;
};

wilton.Console.prototype.append = function(level, logger, message) {
    try {
        if (message instanceof Error) {
            message = message.toString() + "\n" + message.stack;
        } else if ("string" !== typeof (message)) {
            try {
                message = JSON.stringify(message);
            } catch (e) {
                message = "" + message;
            }
        }
        this.jni.appendLog(level, logger, message);
    } catch (e) {
        Packages.java.lang.System.out.println("===LOGGER ERROR:");
        Packages.java.lang.System.out.println(e.toString() + "\n" + e.stack);
        Packages.java.lang.System.out.println("===LOGGER ERROR END:");
    }
};

wilton.Console.prototype.log = function (message) {
    this.append("DEBUG", "wilton", message);
};

wilton.Console.prototype.info = function (message) {
    this.append("INFO", "wilton", message);
};

wilton.Console.prototype.warn = function (message) {
    this.append("WARN", "wilton", message);
};

wilton.Console.prototype.error = function(message) {
    this.append("ERROR", "wilton", message);
};

if ("undefined" === typeof (console)) {
    console = new wilton.Console();
}


// Server

wilton.Server = function(conf, onSuccess, onError) {
    try {
        this.jni = Packages.net.wiltonwebtoolkit.HttpServerJni;
        this.gateway = new wilton.Gateway({
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

wilton.Server.prototype.stop = function(onSuccess, onError) {
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
};


// Response

wilton.Response = function(jni, handle) {
    this.jni = jni;
    this.handle = handle;
};

wilton.Response.prototype.send = function (data, metadata, onSuccess, onError) {
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
};

wilton.Response.prototype.sendFile = function (filePath, metadata, onSuccess, onError) {
    try {
        if ("object" === typeof (metadata)) {
            var json = JSON.stringify(metadata);
            this.jni.setResponseMetadata(this.handle, json);
        }
        this.jni.sendFile(this.handle, filePath);
        if ("function" === typeof (onSuccess)) {
            onSuccess();
        }
    } catch (e) {
        if ("function" === typeof (onError)) {
            onError(e);
        }
    }
};


// Gateway

wilton.Gateway = function(conf) {
    this.jni = conf.jni;
    this.gateway = conf.gateway;
    this.callbacks = conf.callbacks;
};

wilton.Gateway.prototype.gatewayCallback = function(requestHandle) {            
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
            req.data = "" + bdata   ;
        }
        var resp = new wilton.Response(this.jni, requestHandle);        
        cb(req, resp);
    } catch (e) {
        console.error(e);
        var rm = JSON.stringify({
            statusCode: 500,
            statusMessage: "Server Error"
        });
        this.jni.setResponseMetadata(requestHandle, rm);
        this.jni.sendResponse(requestHandle, "500: Server Error");        
    }
};


