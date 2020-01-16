// Copyright (c) 2020 LG Electronics, Inc.
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

#include "updater/block/BlockUpdater.h"

#include <limits.h>
#include <stdlib.h>

#include "bootloader/AbsBootloader.h"
#include "util/Logger.h"

BlockUpdater::BlockUpdater()
{
    setClassName("BlockUpdater");
}

BlockUpdater::~BlockUpdater()
{
}

bool BlockUpdater::onInitialization()
{
    return true;
}

bool BlockUpdater::onFinalization()
{
    return true;
}

bool BlockUpdater::deploy(const string& path, PartitionLabel partitionLabel)
{
    Logger::debug(getClassName(), __FUNCTION__, path);

    bool isDelta = path.rfind(".xd3") != string::npos;
    bool isGZipped = path.rfind(".gz") != string::npos;
    int bootSlot = AbsBootloader::getBootloader().getBootSlot();
    string bootSlotStr = (bootSlot == 0) ? "a" : "b";
    string nextSlotStr = (bootSlot == 0) ? "b" : "a";
    string partitionPrefix = "/dev/disk/by-partlabel/";

    switch (partitionLabel) {
    case PartitionLabel_BOOT:
        partitionPrefix += "boot_";
        break;
    case PartitionLabel_SYSTEM:
        partitionPrefix += "system_";
        break;
    case PartitionLabel_NONE:
    default:
        Logger::error(getClassName(), __FUNCTION__, "Unknown partition: " + to_string(partitionLabel));
        return false;
    }
    Logger::debug(getClassName(), __FUNCTION__, partitionPrefix + bootSlotStr + " to " + nextSlotStr);

    char currentPartition[PATH_MAX] = { 0, };
    char nextPartition[PATH_MAX] = { 0, };
    if (realpath((partitionPrefix + bootSlotStr).c_str(), currentPartition) == NULL ||
        realpath((partitionPrefix + nextSlotStr).c_str(), nextPartition) == NULL) {
        Logger::error(getClassName(), __FUNCTION__, string("Get realpath error: ") + strerror(errno));
        return false;
    }
    Logger::debug(getClassName(), __FUNCTION__, string(currentPartition) + " to " + nextPartition);

    string systemCmd;
    if (isDelta) {
        systemCmd = "xdelta3 -S none -d -s " + string(currentPartition) + " " + path + " " + nextPartition;
    } else if (isGZipped) {
        systemCmd = "gunzip -c " + path + " | dd iflag=fullblock oflag=direct status=progress bs=4M of=" + nextPartition + "; sync";
    } else {
        systemCmd = "dd if=" + path + " oflag=direct status=progress bs=4M of=" + nextPartition + "; sync";
    }
    Logger::debug(getClassName(), __FUNCTION__, systemCmd);
    int rc = system(systemCmd.c_str());
    return WIFEXITED(rc) && WEXITSTATUS(rc) == 0;
}

bool BlockUpdater::undeploy()
{
    Logger::debug(getClassName(), __FUNCTION__);

    int bootSlot = AbsBootloader::getBootloader().getBootSlot();
    if (AbsBootloader::getBootloader().setActive(bootSlot)) {
        Logger::error(getClassName(), __FUNCTION__, "setActive error");
        return false;
    }

    Logger::debug(getClassName(), __FUNCTION__, to_string(bootSlot));
    return true;
}

bool BlockUpdater::setReadWriteMode()
{
    Logger::debug(getClassName(), __FUNCTION__);

    return true;
}

bool BlockUpdater::isUpdated()
{
    Logger::debug(getClassName(), __FUNCTION__);

    return true;
}

void BlockUpdater::printDebug()
{
    Logger::debug(getClassName(), __FUNCTION__);
}
