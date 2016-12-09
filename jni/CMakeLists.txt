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
project ( wiltonjs_jni CXX )
set ( ${PROJECT_NAME}_STATICLIB_VERSION 0.7.1-SNAPSHOT )
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
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_mustache )
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
        staticlib_mustache
        staticlib_tinydir
        staticlib_pimpl
        staticlib_serialization
        staticlib_utils
        log4cplus
        soci
        jansson
        libcurl
        libssl
        zlib 
        sqlite3 )
staticlib_pkg_check_modules ( ${PROJECT_NAME}_DEPS_PRIVATE_PC REQUIRED ${PROJECT_NAME}_DEPS_PRIVATE )

# build steps
set_property ( GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE )
add_library ( ${PROJECT_NAME} SHARED
        ${CMAKE_CURRENT_LIST_DIR}/wiltonjs/wiltoncall.cpp 
        ${CMAKE_CURRENT_LIST_DIR}/wiltonjs/wiltonjs_db.cpp 
        ${CMAKE_CURRENT_LIST_DIR}/wiltonjs/wiltonjs_cron.cpp 
        ${CMAKE_CURRENT_LIST_DIR}/wiltonjs/wiltonjs_detail.cpp 
        ${CMAKE_CURRENT_LIST_DIR}/wiltonjs/wiltonjs_httpclient.cpp 
        ${CMAKE_CURRENT_LIST_DIR}/wiltonjs/wiltonjs_logger.cpp 
        ${CMAKE_CURRENT_LIST_DIR}/wiltonjs/wiltonjs_misc.cpp 
        ${CMAKE_CURRENT_LIST_DIR}/wiltonjs/wiltonjs_mustache.cpp 
        ${CMAKE_CURRENT_LIST_DIR}/wiltonjs/wiltonjs_mutex.cpp 
        ${CMAKE_CURRENT_LIST_DIR}/wiltonjs/wiltonjs_server.cpp 
        ${CMAKE_CURRENT_LIST_DIR}/wiltonjs/wiltonjs_shared.cpp 
        ${CMAKE_CURRENT_LIST_DIR}/wiltonjs/wiltonjs_thread.cpp 
        ${CMAKE_CURRENT_LIST_DIR}/wiltonjs_jni.cpp )
target_include_directories ( ${PROJECT_NAME} BEFORE PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${${PROJECT_NAME}_DEPS_PUBLIC_PC_INCLUDE_DIRS} )
target_compile_options ( ${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_DEPS_PUBLIC_PC_CFLAGS_OTHER} )
target_link_libraries ( ${PROJECT_NAME} 
        ${${PROJECT_NAME}_DEPS_PUBLIC_PC_LIBRARIES}
        ${${PROJECT_NAME}_DEPS_PRIVATE_PC_LIBRARIES} )


# maven install

if ( STATICLIB_TOOLCHAIN MATCHES "android_armeabi_[^_]+" )
    set ( ${PROJECT_NAME}_CLASSIFIER "android_armeabi" CACHE INTERNAL "" )
    set ( ${PROJECT_NAME}_PACKAGING "so" CACHE INTERNAL "" )
    set ( ${PROJECT_NAME}_JAR_LIB_DIR "armeabi" CACHE INTERNAL "" )
elseif ( STATICLIB_TOOLCHAIN MATCHES "android_i386_[^_]+" )
    set ( ${PROJECT_NAME}_CLASSIFIER "android_i386" CACHE INTERNAL "" )
    set ( ${PROJECT_NAME}_PACKAGING "so" CACHE INTERNAL "" )
    set ( ${PROJECT_NAME}_JAR_LIB_DIR "x86" CACHE INTERNAL "" )
elseif ( STATICLIB_TOOLCHAIN MATCHES "linux_amd64_[^_]+" )
    set ( ${PROJECT_NAME}_CLASSIFIER "linux_amd64" CACHE INTERNAL "" )
    set ( ${PROJECT_NAME}_PACKAGING "so" CACHE INTERNAL "" )
    set ( ${PROJECT_NAME}_JAR_LIB_DIR "linux_amd64" CACHE INTERNAL "" )
else ( )
    message ( FATAL_ERROR "Unsupported toolchain: [STATICLIB_TOOLCHAIN]" )
endif ( )

add_custom_target ( mvninstall_so
        COMMAND mvn install:install-file
                -Dfile=${CMAKE_SHARED_LIBRARY_PREFIX}${PROJECT_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}
                -DgroupId=net.wiltonwebtoolkit
                -DartifactId=${PROJECT_NAME}
                -Dversion=${${PROJECT_NAME}_STATICLIB_VERSION}
                -Dclassifier=${${PROJECT_NAME}_CLASSIFIER}
                -Dpackaging=${${PROJECT_NAME}_PACKAGING}
        WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}        
        COMMENT "Installing library into local Maven repository ..." )

add_dependencies ( mvninstall_so ${PROJECT_NAME} )

add_custom_target ( mvninstall_jar
        COMMAND ${CMAKE_COMMAND} -E remove_directory lib
        COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_SHARED_LIBRARY_PREFIX}${PROJECT_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}.jar
        COMMAND ${CMAKE_COMMAND} -E make_directory lib/${${PROJECT_NAME}_ADNROID_ARCH_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SHARED_LIBRARY_PREFIX}${PROJECT_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}
                lib/${${PROJECT_NAME}_JAR_LIB_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${PROJECT_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}
        COMMAND jar cf
                ${CMAKE_SHARED_LIBRARY_PREFIX}${PROJECT_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}.jar 
                lib/${${PROJECT_NAME}_JAR_LIB_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${PROJECT_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}
        COMMAND mvn install:install-file
                -Dfile=${CMAKE_SHARED_LIBRARY_PREFIX}${PROJECT_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}.jar
                -DgroupId=net.wiltonwebtoolkit
                -DartifactId=${PROJECT_NAME}
                -Dversion=${${PROJECT_NAME}_STATICLIB_VERSION}
                -Dclassifier=${${PROJECT_NAME}_CLASSIFIER}.so
                -Dpackaging=jar
        WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}        
        COMMENT "Installing library into local Maven repository as JAR ..." )

add_dependencies ( mvninstall_jar ${PROJECT_NAME} )
