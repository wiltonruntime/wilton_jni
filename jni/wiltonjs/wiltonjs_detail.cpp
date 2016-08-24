/* 
 * File:   wiltonjs_detail.cpp
 * Author: alex
 *
 * Created on August 21, 2016, 1:14 PM
 */

#include "wiltonjs/wiltonjs.hpp"

#include "staticlib/utils.hpp"

#include "wilton/wilton.h"

namespace wiltonjs {
namespace detail {

namespace { //anonymous

namespace ss = staticlib::serialization;
namespace su = staticlib::utils;

} // namespace

void throw_wilton_error(char* err, const std::string& msg) {
    wilton_free(err);
    throw WiltonJsException(msg);
}

std::string wrap_wilton_output(char* out, int out_len) {
    std::string res{out, static_cast<std::string::size_type> (out_len)};
    wilton_free(out);
    return res;
}

const std::string& get_json_string(const ss::JsonField& field, const std::string& name) {
    if (ss::JsonType::STRING != field.get_type() || field.get_string().empty()) {
        throw WiltonJsException(TRACEMSG("Invalid '" + name + "' field,"
                " type: [" + ss::stringify_json_type(field.get_type()) + "]," +
                " value: [" + ss::dump_json_to_string(field.get_value()) + "]"));
    }
    return field.get_string();
}

int64_t get_json_handle(const ss::JsonField& field, const std::string& name) {
    if (ss::JsonType::INTEGER != field.get_type()) {
        throw WiltonJsException(TRACEMSG("Invalid '" + name + "' field,"
                " type: [" + ss::stringify_json_type(field.get_type()) + "]," +
                " value: [" + ss::dump_json_to_string(field.get_value()) + "]"));
    }
    return field.get_integer();
}

const ss::JsonValue& get_json_object(
        const ss::JsonField& field, const std::string& name) {
    if (ss::JsonType::OBJECT != field.get_type()) {
        throw WiltonJsException(TRACEMSG("Invalid '" + name + "' field,"
                " type: [" + ss::stringify_json_type(field.get_type()) + "]," +
                " value: [" + ss::dump_json_to_string(field.get_value()) + "]"));
    }
    return field.get_value();    
}

} // namespace
}
