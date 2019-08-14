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

#include "bootloader/AbsBootloader.h"
#include "bootloader/ICAS.h"
#include "bootloader/UBoot.h"
#include "Setting.h"
#include "util/Socket.h"

AbsBootloader& AbsBootloader::getBootloader()
{
    static ICAS icas;
    return icas;
}

AbsBootloader::AbsBootloader()
{
}

AbsBootloader::~AbsBootloader()
{
}

string AbsBootloader::getHawkBitId()
{
    // TODO : distinguish devel or production (refer WEBOS_DISTRO_PRERELEASE)
    bool isDevel = true;

    if (isDevel) {
        if (!Setting::getInstance().getId().empty()) {
            return Setting::getInstance().getId();
        } else {
            return "webOS_undefined";
        }
    } else {
        if (!Socket::getMacAddress("eth0").empty()) {
            return Socket::getMacAddress("eth0");
        } else if (!Socket::getMacAddress("wlan0").empty()) {
            return Socket::getMacAddress("wlan0");
        } else {
            char buff[256];
            stringstream ss;
            FILE* file = popen("echo -n `nyx-cmd DeviceInfo query nduid 2>/dev/null`", "r");
            if (!file) {
                return "undefined";
            }
            while (fgets(buff, sizeof(buff), file)) {
                ss << buff;
            }
            pclose(file);
            return ss.str();
        }
    }
}
