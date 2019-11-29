# @@@LICENSE
#
#      Copyright (c) 2019 LG Electronics, Inc.
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
#
# LICENSE@@@

option(LIBOSTREE "Build with libostree" ON)
option(LIBBOOTCTRL "Build with libbootctrl" OFF)

# Uncomment to override the following variables defined in .bb file.
# set(HAWKBIT_ADDRESS http://10.178.84.116:8080)
# set(HAWKBIT_TOKEN 377b83e10b9f894883e98351875151cb)
webos_add_compiler_flags(ALL -DINCLUDE_WEBOS)
webos_configure_header_files(${CMAKE_CURRENT_SOURCE_DIR})

include(FindPkgConfig)

pkg_check_modules(CURL REQUIRED libcurl)
include_directories(${CURL_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${CURL_CFLAGS_OTHER})

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${GLIB2_CFLAGS_OTHER})

pkg_check_modules(GIO_UNIX2 REQUIRED gio-unix-2.0)
include_directories(${GIO_UNIX2_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${GIO_UNIX2_CFLAGS_OTHER})

pkg_check_modules(GOBJECT2 REQUIRED gobject-2.0)
include_directories(${GOBJECT2_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${GOBJECT2_CFLAGS_OTHER})

pkg_check_modules(LUNASERVICE2 luna-service2)
include_directories(${LUNASERVICE2_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${LUNASERVICE2_CFLAGS_OTHER})

pkg_check_modules(LUNASERVICE2CPP luna-service2++)
include_directories(${LUNASERVICE2CPP_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${LUNASERVICE2CPP_CFLAGS_OTHER})

pkg_check_modules(PBNJSON_C REQUIRED pbnjson_c)
include_directories(${PBNJSON_C_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${PBNJSON_C_CFLAGS_OTHER})

pkg_check_modules(PBNJSON_CPP REQUIRED pbnjson_cpp)
include_directories(${PBNJSON_CPP_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${PBNJSON_CPP_CFLAGS_OTHER})

pkg_check_modules(PMLOG PmLogLib)
include_directories(${PMLOG_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${PMLOG_CFLAGS_OTHER})

pkg_check_modules(BOOST Boost)
include_directories(${Boost_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${Boost_CFLAGS_OTHER})

if (LIBOSTREE)
    pkg_check_modules(OSTREE REQUIRED ostree-1)
    include_directories(${OSTREE_INCLUDE_DIRS})
    webos_add_compiler_flags(ALL ${OSTREE_CFLAGS_OTHER} -DLIBOSTREE -DUBOOT)
endif()
if (LIBBOOTCTRL)
    webos_add_compiler_flags(ALL -DLIBBOOTCTRL)
endif()

# Environment
set(BIN_NAME swupdater)
file(GLOB_RECURSE SRC_CPP ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
if (NOT LIBOSTREE)
    list(REMOVE_ITEM SRC_CPP ${CMAKE_CURRENT_SOURCE_DIR}/updater/ostree/OSTree.cpp)
endif()
if (NOT LIBBOOTCTRL)
    list(REMOVE_ITEM SRC_CPP ${CMAKE_CURRENT_SOURCE_DIR}/updater/dd/DDCommand.cpp)
endif()
file(GLOB_RECURSE SRC_C ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

webos_add_compiler_flags(ALL CXX -std=c++0x)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external)
add_executable(${BIN_NAME} ${SRC_CPP} ${SRC_C})

# Link
set(LIBS
    ${CURL_LDFLAGS}
    ${GLIB2_LDFLAGS}
    ${GIO_UNIX2_LDFLAGS}
    ${GOBJECT2_LDFLAGS}
    ${LUNASERVICE2_LDFLAGS}
    ${LUNASERVICE2CPP_LDFLAGS}
    ${PMLOG_LDFLAGS}
    ${PBNJSON_C_LDFLAGS}
    ${PBNJSON_CPP_LDFLAGS}
    ${Boost_LDFLAGS}
)
if (LIBOSTREE)
    set (LIBS ${LIBS} ${OSTREE_LDFLAGS})
endif()
if (LIBBOOTCTRL)
    set (LIBS ${LIBS} -labctl -lboot_control)
endif()
target_link_libraries(${BIN_NAME} ${LIBS})

# Install
install(TARGETS ${BIN_NAME} DESTINATION ${WEBOS_INSTALL_SBINDIR})
webos_build_configured_file(files/launch/swupdater.service SYSCONFDIR systemd/system)
