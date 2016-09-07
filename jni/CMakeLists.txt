# Copyright 2016, alex at staticlibs.net
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required ( VERSION 2.8.12 )

# toolchain setup
if ( NOT DEFINED STATICLIB_DEPS )
    set ( STATICLIB_DEPS ${CMAKE_CURRENT_LIST_DIR}/../../../staticlibs CACHE INTERNAL "" )    
endif ( )
if ( NOT DEFINED WILTON_DEPS )
    set ( WILTON_DEPS ${CMAKE_CURRENT_LIST_DIR}/../.. CACHE INTERNAL "" )
endif ( )
set ( STATICLIB_TOOLCHAIN linux_amd64_gcc CACHE STRING "toolchain triplet" )
if ( NOT DEFINED STATICLIB_CMAKE )
    set ( STATICLIB_CMAKE ${STATICLIB_DEPS}/cmake CACHE INTERNAL "" )    
endif ( )
set ( CMAKE_TOOLCHAIN_FILE ${STATICLIB_CMAKE}/toolchains/${STATICLIB_TOOLCHAIN}.cmake CACHE INTERNAL "" )

# project
project ( wilton_jni CXX )
set ( ${PROJECT_NAME}_STATICLIB_VERSION 1.0 )
set ( ${PROJECT_NAME}_DESCRIPTION "Wilton Web Toolkit JNI library" )
set ( ${PROJECT_NAME}_URL https://github.com/wilton-web-toolkit/wilton.js )
include ( ${STATICLIB_CMAKE}/staticlibs_common.cmake )
staticlib_enable_deplibs_cache ( )

# options
set ( staticlib_httpserver_USE_LOG4CPLUS ON CACHE BOOL "Use log4cplus lib for logging" )
set ( staticlib_httpserver_USE_OPENSSL ON CACHE BOOL "Use OpenSSL lib for https" )

# dependencies
if ( NOT STATICLIB_TOOLCHAIN MATCHES "linux_[^_]+_[^_]+" )    
    staticlib_add_subdirectory ( ${STATICLIB_DEPS}/external_zlib )
    staticlib_add_subdirectory ( ${STATICLIB_DEPS}/external_jansson )
    staticlib_add_subdirectory ( ${STATICLIB_DEPS}/external_log4cplus )
    staticlib_add_subdirectory ( ${STATICLIB_DEPS}/external_openssl )
    staticlib_add_subdirectory ( ${STATICLIB_DEPS}/external_curl )
    staticlib_add_subdirectory ( ${STATICLIB_DEPS}/external_sqlite )
endif ( )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/external_mstch )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/external_soci )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_jni )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_config )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_io )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_ranges )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_endian )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_httpserver )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_utils )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_serialization )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_pimpl )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_orm )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_unzip )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_httpclient )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_cron )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_tinydir )
add_subdirectory ( ${WILTON_DEPS}/libwilton ${CMAKE_BINARY_DIR}/libwilton )

# dependencies
set ( ${PROJECT_NAME}_DEPS_PUBLIC
        staticlib_jni
        staticlib_config
        wilton )
staticlib_pkg_check_modules ( ${PROJECT_NAME}_DEPS_PUBLIC_PC REQUIRED ${PROJECT_NAME}_DEPS_PUBLIC )
set ( ${PROJECT_NAME}_DEPS_PRIVATE
        staticlib_io
        staticlib_ranges
        staticlib_httpserver
        staticlib_unzip
        staticlib_orm
        staticlib_httpclient
        staticlib_cron
        staticlib_tinydir
        staticlib_pimpl
        staticlib_serialization
        staticlib_utils
        log4cplus
        mstch
        soci
        jansson
        libcurl
        libssl
        libcrypto
        zlib 
        sqlite3 )
staticlib_pkg_check_modules ( ${PROJECT_NAME}_DEPS_PRIVATE_PC REQUIRED ${PROJECT_NAME}_DEPS_PRIVATE )

# build steps
set_property ( GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE )
add_library ( ${PROJECT_NAME} SHARED
        ${CMAKE_CURRENT_LIST_DIR}/wiltonjs/wiltoncall.cpp 
        ${CMAKE_CURRENT_LIST_DIR}/wiltonjs/wiltonjs_db.cpp 
        ${CMAKE_CURRENT_LIST_DIR}/wiltonjs/wiltonjs_detail.cpp 
        ${CMAKE_CURRENT_LIST_DIR}/wiltonjs/wiltonjs_httpclient.cpp 
        ${CMAKE_CURRENT_LIST_DIR}/wiltonjs/wiltonjs_logger.cpp 
        ${CMAKE_CURRENT_LIST_DIR}/wiltonjs/wiltonjs_mustache.cpp 
        ${CMAKE_CURRENT_LIST_DIR}/wiltonjs/wiltonjs_server.cpp 
        ${CMAKE_CURRENT_LIST_DIR}/wiltonjs_jni.cpp )
target_include_directories ( ${PROJECT_NAME} BEFORE PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${${PROJECT_NAME}_DEPS_PUBLIC_PC_INCLUDE_DIRS} )
target_compile_options ( ${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_DEPS_PUBLIC_PC_CFLAGS_OTHER} )
target_link_libraries ( ${PROJECT_NAME} 
        ${${PROJECT_NAME}_DEPS_PUBLIC_PC_LIBRARIES}
        ${${PROJECT_NAME}_DEPS_PRIVATE_PC_LIBRARIES} )
