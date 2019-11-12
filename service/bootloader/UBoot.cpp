// Copyright (c) 2019 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include "bootloader/UBoot.h"

#include <string.h>

UBoot::UBoot()
{
}

UBoot::~UBoot()
{
}

void UBoot::setEnv(const string& key, const string& value)
{
    string command = "/sbin/fw_setenv " + key + " " + value;

    FILE* file = popen(command.c_str(), "r");
    if (!file) {
        return;
    }

    pclose(file);
}

string UBoot::getEnv(const string& key)
{
    char buff[256];
    stringstream ss;
    string command = "/sbin/fw_printenv -n " + key + " 2>/dev/null";

    FILE* file = popen(command.c_str(), "r");
    if (!file) {
        return "";
    }

    while (fgets(buff, sizeof(buff), file)) {
        // remove trailing newline
        buff[strcspn(buff, "\n")] = 0;
        ss << buff;
    }

    pclose(file);
    return ss.str();
}

void UBoot::notifyUpdate()
{
    // 'bootcount' is increased only when 'upgrade_available' is set.
    setEnv("upgrade_available", "1");
    setEnv("bootcount", "0");
    setEnv("rollback", "0");
}

void UBoot::setRebootOK()
{
    setEnv("upgrade_available", "0");
    setEnv("bootcount", "0");
    // Do not set 'rollback' to 0.
    // 'rollback = 1' means, update is failed and booted into alternative deployment.
    // so next boot up, boot directly into alternative deployment.
}
