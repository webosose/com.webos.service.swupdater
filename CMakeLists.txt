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

cmake_minimum_required(VERSION 2.8.7)

project(com.webos.service.swupdater)

include(webOS/webOS)

# modules
webos_modules_init(1 1 1 QUALIFIER RC4)
webos_component(1 0 0)

add_subdirectory(service)
add_subdirectory(application)

# bus
webos_build_system_bus_files()

