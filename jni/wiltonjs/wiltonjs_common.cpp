/* 
 * File:   wiltonjs_common.cpp
 * Author: alex
 *
 * Created on August 21, 2016, 1:14 PM
 */

#include "wiltonjs/wiltonjs.hpp"

#include "wilton/wilton.h"

namespace wiltonjs {
namespace detail {

void throw_wilton_error(char* err, const std::string& msg) {
    wilton_free(err);
    throw WiltonJsException(msg);
}

std::string wrap_wilton_output(char* out, int out_len) {
    std::string res{out, static_cast<std::string::size_type> (out_len)};
    wilton_free(out);
    return res;
}

} // namespace
}
