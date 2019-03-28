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

#include <core/install/InstallAction.h>

InstallAction::InstallAction()
    : AbsAction()
    , m_download(ScheduleType_UNKNOWN)
    , m_update(ScheduleType_UNKNOWN)
{
    setType(ActionType_INSTALL);
}

InstallAction::~InstallAction()
{
}

ScheduleType InstallAction::toEnum(const string& type)
{
    if (type == "attempt")
        return ScheduleType_ATTEMPT;
    if (type == "forced")
        return ScheduleType_FORCED;
    if (type == "skip")
        return ScheduleType_SKIP;

    return ScheduleType_UNKNOWN;
}

string InstallAction::toString(enum ScheduleType& type)
{
    switch (type) {
    case ScheduleType_SKIP:
        return "skip";
    case ScheduleType_ATTEMPT:
        return "attempt";
    case ScheduleType_FORCED:
        return "forced";
    case ScheduleType_UNKNOWN:
        return "unknown";
    }
    return "unknown";
}

bool InstallAction::fromJson(const JValue& json)
{
    ISerializable::fromJson(json);
    AbsAction::fromJson(json);
    if (json.hasKey("actionHistory")) {
        JValue actionHistory = json["actionHistory"];
        if (actionHistory.hasKey("status") && actionHistory["status"].isString()) {
            m_historyStatus = actionHistory["status"].asString();
        }
        if (actionHistory.hasKey("messages") && actionHistory["messages"].isArray()) {
            for (JValue message : actionHistory["messages"].items()) {
                m_historyMessages.push_back(message.asString());
            }
        }
    }

    if (!json.hasKey("deployment")) {
        return false;
    }
    JValue deployment = json["deployment"];
    if (deployment.hasKey("chunks") && deployment["chunks"].isArray()) {
        for (JValue chunkJson : deployment["chunks"].items()) {
            SoftwareModule chunk;
            chunk.fromJson(chunkJson);
            m_chunks.push_back(chunk);
        }
    }
    if (deployment.hasKey("download") && deployment["download"].isString()) {
        string download = deployment["download"].asString();
        m_download = toEnum(download);
    }
    if (deployment.hasKey("update") && deployment["update"].isString()) {
        string update = deployment["update"].asString();
        m_update = toEnum(update);
    }
    if (deployment.hasKey("maintenanceWindow") && deployment["maintenanceWindow"].isString()) {
        m_maintenanceWindow = deployment["maintenanceWindow"].asString();
    }
    return true;
}
