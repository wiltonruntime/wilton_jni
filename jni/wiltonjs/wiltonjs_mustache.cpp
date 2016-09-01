/* 
 * File:   wiltonjs_mustache.cpp
 * Author: alex
 *
 * Created on August 21, 2016, 1:14 PM
 */

#include "wiltonjs/wiltonjs.hpp"

#include "staticlib/serialization.hpp"

#include "wilton/wilton.h"

namespace wiltonjs {

namespace { // anonymous

namespace ss = staticlib::serialization;

const std::string EMPTY_STRING = "";

} // namespace

std::string mustache_render(const std::string& data, void*) {
    // parse json
    ss::JsonValue json = ss::load_json_from_string(data);
    auto rtemplate = std::ref(EMPTY_STRING);
    std::string values = EMPTY_STRING;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("template" == name) {
            rtemplate = detail::get_json_string(fi, "template");
        } else if ("values" == name) {
            values = ss::dump_json_to_string(fi.value());
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (rtemplate.get().empty()) throw WiltonJsException(TRACEMSG(
            "Required parameter 'level' not specified, data: [" + data + "]"));
    const std::string& templade = rtemplate.get();
    if (values.empty()) {
        values = "{}";
    }
    // call wilton
    char* out;
    int out_len;
    char* err = wilton_render_mustache(templade.c_str(), templade.length(),
            values.c_str(), values.length(), std::addressof(out), std::addressof(out_len));
    if (nullptr != err) detail::throw_wilton_error(err, TRACEMSG(std::string(err) + 
            "\nMustache render error for input data: [" + data + "]"));
    return detail::wrap_wilton_output(out, out_len);
}
    
} // namespace
