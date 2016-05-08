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

# project
project ( wilton_jni CXX )
set ( ${PROJECT_NAME}_STATICLIB_VERSION 1.0 )
set ( ${PROJECT_NAME}_DESCRIPTION "Wilton Web Toolkit JNI library" )
set ( ${PROJECT_NAME}_URL https://github.com/wilton-web-toolkit/wilton_jni )

# enhanced version of pkg_check_modules macro with PKG_CONFIG_PATH logic
# PKG_CONFIG_PATH handling through CMAKE_PREFIX_PATH was added in newer versions of CMake
macro ( ${PROJECT_NAME}_pkg_check_modules _out_var_name _modifier _modules_list_var_name )
    find_package ( PkgConfig )
    if ( WIN32 )
        set ( PATHENV_SEPARATOR ";" )
    else ( )
        set ( PATHENV_SEPARATOR ":" )
    endif ( )
    set (_pkgconfig_path $ENV{PKG_CONFIG_PATH} )
    if ( STATICLIB_USE_DEPLIBS_CACHE )
        set ( ENV{PKG_CONFIG_PATH} "${STATICLIB_DEPLIBS_CACHE_DIR}/pkgconfig${PATHENV_SEPARATOR}$ENV{PKG_CONFIG_PATH}" )
    endif ( )
    set ( ENV{PKG_CONFIG_PATH} "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/pkgconfig${PATHENV_SEPARATOR}$ENV{PKG_CONFIG_PATH}" )
    pkg_check_modules ( ${_out_var_name} ${_modifier} ${${_modules_list_var_name}} )
    set ( ENV{PKG_CONFIG_PATH} ${_pkgconfig_path} )
endmacro ( )

# standalone build
if ( NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY )
    set ( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
endif ( )
if ( NOT DEFINED STATICLIB_TOOLCHAIN )
    staticlib_cbindhttp_add_subdirectory ( ${CMAKE_CURRENT_LIST_DIR}/../staticlib_config )
    staticlib_cbindhttp_add_subdirectory ( ${CMAKE_CURRENT_LIST_DIR}/../staticlib_utils )
    staticlib_cbindhttp_add_subdirectory ( ${CMAKE_CURRENT_LIST_DIR}/../staticlib_httpserver )
    staticlib_cbindhttp_add_subdirectory ( ${CMAKE_CURRENT_LIST_DIR}/../staticlib_serialization )
endif ( )

# dependencies
set ( ${PROJECT_NAME}_DEPS
        jansson
        staticlib_config
        staticlib_utils
        staticlib_httpserver
        staticlib_serialization )
wilton_c_pkg_check_modules ( ${PROJECT_NAME}_DEPS_PC REQUIRED ${PROJECT_NAME}_DEPS )

# build steps
file ( GLOB_RECURSE ${PROJECT_NAME}_SRC ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp )
set ( ${PROJECT_NAME}_HEADERS ${CMAKE_CURRENT_LIST_DIR}/include/wilton/wilton_c.h )
file ( GLOB_RECURSE ${PROJECT_NAME}_HEADERS_PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/*.hpp )
add_library ( ${PROJECT_NAME} STATIC 
        ${${PROJECT_NAME}_SRC} 
        ${${PROJECT_NAME}_HEADERS}
        ${${PROJECT_NAME}_HEADERS_PRIVATE} )

source_group ( "src" FILES ${${PROJECT_NAME}_SRC} )
source_group ( "include" FILES ${${PROJECT_NAME}_HEADERS} )
source_group ( "include_private" FILES ${${PROJECT_NAME}_HEADERS_PRIVATE} )
target_include_directories ( ${PROJECT_NAME} BEFORE PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${${PROJECT_NAME}_DEPS_PC_INCLUDE_DIRS} )
target_compile_options ( ${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_DEPS_PC_CFLAGS_OTHER} )
target_link_libraries ( ${PROJECT_NAME} ${${PROJECT_NAME}_DEPS_PC_LIBRARIES} )

