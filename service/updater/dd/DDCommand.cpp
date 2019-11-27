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

#include "updater/dd/DDCommand.h"

#include <limits.h>
#include <stdlib.h>

#include "bootloader/AbsBootloader.h"
#include "util/Logger.h"

DDCommand::DDCommand()
{
    setClassName("DDCommand");
}

DDCommand::~DDCommand()
{
}

bool DDCommand::onInitialization()
{
    return true;
}

bool DDCommand::onFinalization()
{
    return true;
}

bool DDCommand::deploy(const string& path, PartitionLabel partLabel)
{
    Logger::debug(getClassName(), __FUNCTION__);

    int currentSlot = AbsBootloader::getBootloader().getCurrentSlot();
    string slotSuffix = (currentSlot == 0) ? "b" : "a";
    string partLink = "/dev/disk/by-partlabel/";

    switch (partLabel) {
    case PartitionLabel_BOOT:
        partLink += "boot_" + slotSuffix;
        break;
    case PartitionLabel_SYSTEM:
        partLink += "system_" + slotSuffix;
        break;
    case PartitionLabel_NONE:
    default:
        Logger::error(getClassName(), __FUNCTION__, "Unknown partition: " + to_string(partLabel));
        return false;
    }
    Logger::debug(getClassName(), __FUNCTION__, partLink);

    char destPath[PATH_MAX] = { 0, };
    if (realpath(partLink.c_str(), destPath) == NULL) {
        Logger::error(getClassName(), __FUNCTION__, string("Get realpath error: ") + strerror(errno));
        return false;
    }
    Logger::debug(getClassName(), __FUNCTION__, destPath);

    string systemCmd;
    if (path.substr(path.find_last_of(".") + 1) == "gz") // compressed case
        systemCmd = "gunzip -c " + path + " | dd iflag=fullblock oflag=direct status=progress bs=4M of=" + destPath + "; sync";
    else
        systemCmd = "dd if=" + path + " oflag=direct status=progress bs=4M of=" + destPath + "; sync";
    Logger::debug(getClassName(), __FUNCTION__, systemCmd);
    int rc = system(systemCmd.c_str());
    return WIFEXITED(rc) && WEXITSTATUS(rc) == 0;
}

bool DDCommand::undeploy()
{
    Logger::debug(getClassName(), __FUNCTION__);

    int currentSlot = AbsBootloader::getBootloader().getCurrentSlot();
    if (AbsBootloader::getBootloader().setActiveBootSlot(currentSlot)) {
        Logger::error(getClassName(), __FUNCTION__, "setActiveBootSlot error");
        return false;
    }

    Logger::debug(getClassName(), __FUNCTION__, to_string(currentSlot));
    return true;
}

bool DDCommand::setReadWriteMode()
{
    Logger::debug(getClassName(), __FUNCTION__);

    return true;
}

bool DDCommand::isUpdated()
{
    Logger::debug(getClassName(), __FUNCTION__);

    return true;
}

void DDCommand::printDebug()
{
    Logger::debug(getClassName(), __FUNCTION__);
}
