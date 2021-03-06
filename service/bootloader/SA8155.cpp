// Copyright (c) 2019-2020 LG Electronics, Inc.
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

#include "bootloader/SA8155.h"

#include "util/Logger.h"

#if !defined(LIBABCTL) // for desktop build
    int libabctl_getBootSlot()
    {
        return 0;
    }
    int libabctl_SetBootSuccess()
    {
        return 0;
    }
    int libabctl_setActive(int slot)
    {
        return 0;
    }
#else
    #include <libabctl.h>
#endif

SA8155::SA8155()
{
    setClassName("SA8155");
}

SA8155::~SA8155()
{
}

void SA8155::setEnv(const string& key, const string& value)
{
    // nothing
}

string SA8155::getEnv(const string& key)
{
    return "";
}


void SA8155::notifyUpdate()
{
    Logger::debug(getClassName(), __FUNCTION__);

    int bootSlot = getBootSlot();
    int nextSlot = (bootSlot == 1) ? 0 : 1;
    setActive(nextSlot);
}

void SA8155::setBootSuccess()
{
    Logger::debug(getClassName(), __FUNCTION__);

    libabctl_SetBootSuccess();
}

int SA8155::getBootSlot()
{
    Logger::debug(getClassName(), __FUNCTION__);

    int bootSlot = libabctl_getBootSlot();
    Logger::debug(getClassName(), __FUNCTION__, to_string(bootSlot));
    return bootSlot;
}

int SA8155::setActive(int slot)
{
    Logger::debug(getClassName(), __FUNCTION__, to_string(slot));

    return libabctl_setActive(slot);
}
