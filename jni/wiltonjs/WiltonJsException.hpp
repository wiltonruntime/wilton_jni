/* 
 * File:   WiltonJsException.hpp
 * Author: alex
 *
 * Created on August 22, 2016, 8:52 AM
 */

#ifndef WILTONJS_WILTONJSEXCEPTION_HPP
#define	WILTONJS_WILTONJSEXCEPTION_HPP

#include <exception>
#include <string>

// http://stackoverflow.com/a/18387764/314015
#if !defined(_MSC_VER) || _MSC_VER >= 1900
#define WILTONJS_NOEXCEPT noexcept
#else
#define WILTONJS_NOEXCEPT
#endif // _MSC_VER

namespace wiltonjs {

class WiltonJsException : public std::exception {
protected:
    /**
     * Error message
     */
    std::string message;

public:
    /**
     * Default constructor
     */
    WiltonJsException() = default;

    /**
     * Constructor with message
     * 
     * @param msg error message
     */
    WiltonJsException(std::string message) :
    message(std::move(message)) { }

    /**
     * Returns error message
     * 
     * @return error message
     */
    virtual const char* what() const WILTONJS_NOEXCEPT {
        return message.c_str();
    }
};


} // namespace

#endif	/* WILTONJS_WILTONJSEXCEPTION_HPP */

