/* 
 * File:   wiltoncall.hpp
 * Author: alex
 *
 * Created on August 21, 2016, 8:08 PM
 */

#ifndef WILTONJS_WILTONCALL_HPP
#define	WILTONJS_WILTONCALL_HPP

#include <functional>
#include <string>

#include "wiltonjs/WiltonJsException.hpp"

namespace wiltonjs {

void put_wilton_function(const std::string& name, 
        std::function<std::string(const std::string&, void*)> fun);

std::string invoke_wilton_function(const std::string& name, const std::string& data = "", 
        void* object = nullptr);

} // namespace

#endif	/* WILTONJS_WILTONCALL_HPP */

