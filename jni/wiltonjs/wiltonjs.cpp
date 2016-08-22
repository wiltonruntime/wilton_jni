/* 
 * File:   wiltonjs.cpp
 * Author: alex
 *
 * Created on August 21, 2016, 1:14 PM
 */

#include "wiltonjs/wiltonjs.hpp"

#include <mutex>
#include <unordered_map>

#include "staticlib/serialization.hpp"

#include "wilton/wilton.h"

namespace wiltonjs {

namespace { // anonymous

namespace ss = staticlib::serialization;

} // namespace

std::string render_mustache(const std::string& data, void*) {
    ss::JsonValue data_json = ss::load_json_from_string(data);
    const std::string& templade = data_json.get("template").get_string();
    std::string values = ss::dump_json_to_string(data_json.get("values"));    
    char* out;
    int out_len;
    char* err = wilton_render_mustache(templade.c_str(), templade.length(),
            values.c_str(), values.length(), std::addressof(out), std::addressof(out_len));
    if (nullptr == err) {
        std::string res{out, static_cast<size_t>(out_len)};
        wilton_free(out);
        return res;
    } else {
        std::string err_str{err};
        wilton_free(err);
        throw WiltonJsException(TRACEMSG(err_str + "\nMustache render error"
                " for input data: [" + data + "]"));
    }
}
    
} // namespace
