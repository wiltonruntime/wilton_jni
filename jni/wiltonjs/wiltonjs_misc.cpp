/* 
 * File:   wilton_misc.cpp
 * Author: alex
 *
 * Created on October 4, 2016, 8:13 PM
 */

#include "wiltonjs/wiltonjs.hpp"

#include "wilton/wilton.h"

namespace wiltonjs {

namespace { // anonymous

namespace ss = staticlib::serialization;

} // namespace

std::string sleep_millis(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t millis = -1;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("millis" == name) {
            millis = detail::get_json_handle(fi);
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == millis) throw WiltonJsException(TRACEMSG(
            "Required parameter 'millis' not specified, data: [" + data + "]"));
    // call wilton
    char* err = wilton_sleep_millis(static_cast<int>(millis));
    if (nullptr != err) {
        detail::throw_wilton_error(err, TRACEMSG(std::string(err) +
                "\nsleep_millis error for input data: [" + data + "]"));
    }
    return "{}";
}

} // namespace

