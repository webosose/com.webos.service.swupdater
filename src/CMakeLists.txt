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

# Environment
set(BIN_NAME swupdater)
file(GLOB_RECURSE SRC ${PROJECT_SOURCE_DIR}/src/*.cpp)

#if (NOT INCLUDE_WEBOS)
#    file(GLOB_RECURSE REMOVE_SRC ${PROJECT_SOURCE_DIR}/src/service/webos/*.cpp)
#    list(REMOVE_ITEM SRC ${REMOVE_SRC})
#endif()

# Compile
#if (INCLUDE_WEBOS)
    webos_add_compiler_flags(ALL CXX -std=c++0x)
#else()
#    webos_add_compiler_flags(ALL CXX -std=c++0x)
#endif()

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)
add_executable(${BIN_NAME} ${SRC})

# Link
set(LIBS
    -lswupdate
    ${CURL_LDFLAGS}
    ${GLIB2_LDFLAGS}
    ${GIO_UNIX2_LDFLAGS}
    ${GOBJECT2_LDFLAGS}
    ${LUNASERVICE2_LDFLAGS}
    ${LUNASERVICE2CPP_LDFLAGS}
    ${PMLOG_LDFLAGS}
    ${PBNJSON_C_LDFLAGS}
    ${PBNJSON_CPP_LDFLAGS}
)
target_link_libraries(${BIN_NAME} ${LIBS})

# Install
install(TARGETS ${BIN_NAME} DESTINATION ${WEBOS_INSTALL_SBINDIR})
webos_build_configured_file(files/launch/swupdater.service SYSCONFDIR systemd/system/webos-cbd.target.wants)
