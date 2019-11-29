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

#include "bootloader/SA8155.h"
#include "util/Logger.h"

#if !defined(LIBBOOTCTRL) // for desktop build
    // (abctl) returns current slot number from which device booted.
    int libabctl_getBootSlot()
    {
        return 0;
    }

    // (abctl) marks the success flag for the slot number specified.
    int libabctl_SetBootSuccess()
    {
        return 0;
    }

    // (libbootctrl) Boot control HAL struct
    typedef struct boot_control_module {
        int (*getCurrentSlot)(struct boot_control_module* module);
        int (*markBootSuccessful)(struct boot_control_module* module);
        int (*setActiveBootSlot)(struct boot_control_module* module, unsigned slot);
    } boot_control_module_t;

    int getCurrentSlot(struct boot_control_module* module)
    {
        return 0;
    }

    int markBootSuccessful(struct boot_control_module* module)
    {
        return 0;
    }

    int setActiveBootSlot(struct boot_control_module* module, unsigned slot)
    {
        return 0;
    }

    boot_control_module_t HAL_MODULE_INFO_SYM = {
        .getCurrentSlot = getCurrentSlot,
        .markBootSuccessful = markBootSuccessful,
        .setActiveBootSlot = setActiveBootSlot,
    };

#else
    #include <hardware/boot_control.h>
    #include <libabctl.h>

    extern boot_control_module_t HAL_MODULE_INFO_SYM;
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

    int currentSlot = getCurrentSlot();
    int nextSlot = (currentSlot == 1) ? 0 : 1;
    setActiveBootSlot(nextSlot);
}

void SA8155::setRebootOK()
{
    Logger::debug(getClassName(), __FUNCTION__);

    // libboot_control
    // boot_control_module_t* module = &HAL_MODULE_INFO_SYM;
    // if (module->markBootSuccessful(module) != 0) {
    //     Logger::error(getClassName(), __FUNCTION__);
    // }
    libabctl_SetBootSuccess();
}

int SA8155::getCurrentSlot()
{
    Logger::debug(getClassName(), __FUNCTION__);

    // libboot_control's 'getCurrentSlot' does not work on current build. Always retuns 0.
    // boot_control_module_t* module = &HAL_MODULE_INFO_SYM;
    // int currentSlot = module->getCurrentSlot(module);
    int currentSlot = libabctl_getBootSlot();
    Logger::debug(getClassName(), __FUNCTION__, to_string(currentSlot));
    return currentSlot;
}

int SA8155::setActiveBootSlot(int slot)
{
    Logger::debug(getClassName(), __FUNCTION__, to_string(slot));

    // libabctl_setActive() was deleted. So use libboot_control.
    boot_control_module_t* module = &HAL_MODULE_INFO_SYM;
    int rc = 0;
    if ((rc = module->setActiveBootSlot(module, slot)) != 0) {
        Logger::error(getClassName(), __FUNCTION__);
    }
    return rc;
}
