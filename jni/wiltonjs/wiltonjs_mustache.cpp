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

} // namespace

std::string mustache_render(const std::string& data, void*) {
    ss::JsonValue data_json = ss::load_json_from_string(data);
    const std::string& templade = data_json["template"].as_string();
    std::string values = ss::dump_json_to_string(data_json["values"]);
    char* out;
    int out_len;
    char* err = wilton_render_mustache(templade.c_str(), templade.length(),
            values.c_str(), values.length(), std::addressof(out), std::addressof(out_len));
    if (nullptr != err) detail::throw_wilton_error(err, TRACEMSG(std::string(err) + 
            "\nMustache render error for input data: [" + data + "]"));
    return detail::wrap_wilton_output(out, out_len);
}
    
} // namespace
