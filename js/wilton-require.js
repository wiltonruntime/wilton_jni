/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */


if ("undefined" !== typeof (define) || "undefined" !== typeof (require)) {
    throw new Error("RequireJS is already loaded");
}

define = null;
require = null;

(function () {
    var prefix = ("undefined" !== typeof (requirejs) &&
            "undefined" !== typeof (requirejs.config) &&
            "undefined" !== typeof (requirejs.config.baseUrl)
            ) ? requirejs.config.baseUrl : "";

    if (prefix.length > 0 && "/" !== prefix[prefix.length - 1]) {
        prefix += "/";
    }

    var curfun = null;

    define = function (fun) {
        curfun = fun;
    };
    
    // unlimited cache
    var cache = new Packages.java.util.concurrent.ConcurrentHashMap();

    require = function (modules, cb) {
        var args = [];
        for (var i = 0; i < modules.length; i++) {
            var path = prefix + modules[i] + ".js";
            var cached = cache.get(path);
            if (null === cached) {
                load(path);
                var mod = curfun();
                if ("undefined" === typeof (mod) || null === mod) {
                    throw new Error("Invalid module returned for path: [" + path + "]");
                }
                cache.put(path, mod);
                cached = mod;
            }
            args.push(cached);
        }
        cb.apply(null, args);
    };
}());
