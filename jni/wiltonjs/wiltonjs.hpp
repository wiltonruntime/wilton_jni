/* 
 * File:   wiltonjs.hpp
 * Author: alex
 *
 * Created on August 21, 2016, 1:15 PM
 */

#ifndef WILTONJS_WILTONJS_HPP
#define	WILTONJS_WILTONJS_HPP

#include <string>

#include "wiltonjs/WiltonJsException.hpp"

namespace wiltonjs {

std::string render_mustache(const std::string& data, void* object);

} // namespace

#endif	/* WILTONJS_WILTONJS_HPP */

