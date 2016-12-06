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

const std::string& get_json_string(const ss::JsonField& field) {
    if (ss::JsonType::STRING != field.type() || field.as_string().empty()) {
        throw WiltonJsException(TRACEMSG("Invalid '" + field.name() + "' field,"
                " type: [" + ss::stringify_json_type(field.type()) + "]," +
                " value: [" + ss::dump_json_to_string(field.value()) + "]"));
    }
    return field.as_string();
}

int64_t get_json_int(const ss::JsonField& field) {
    if (ss::JsonType::INTEGER != field.type()) {
        throw WiltonJsException(TRACEMSG("Invalid '" + field.name() + "' field,"
                " type: [" + ss::stringify_json_type(field.type()) + "]," +
                " value: [" + ss::dump_json_to_string(field.value()) + "]"));
    }
    return field.as_int64();
}

bool get_json_bool(const staticlib::serialization::JsonField& field) {
    if (ss::JsonType::BOOLEAN != field.type()) {
        throw WiltonJsException(TRACEMSG("Invalid '" + field.name() + "' field,"
                " type: [" + ss::stringify_json_type(field.type()) + "]," +
                " value: [" + ss::dump_json_to_string(field.value()) + "]"));
    }
    return field.as_bool();
}

const ss::JsonValue& get_json_object(
        const ss::JsonField& field) {
    if (ss::JsonType::OBJECT != field.type()) {
        throw WiltonJsException(TRACEMSG("Invalid '" + field.name() + "' field,"
                " type: [" + ss::stringify_json_type(field.type()) + "]," +
                " value: [" + ss::dump_json_to_string(field.value()) + "]"));
    }
    return field.value();    
}

} // namespace
}
