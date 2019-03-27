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

#ifndef CORE_ACTION_H_
#define CORE_ACTION_H_

#include <list>
#include <map>
#include <pbnjson.hpp>
#include <string>

#include "Chunk.h"
#include "interface/ISerializable.h"

using namespace pbnjson;
using namespace std;

enum ActionType {
    ActionType_NONE,
    ActionType_INSTALL,
    ActionType_CANCEL,
};

enum ScheduleType {
    ScheduleType_UNKNOWN,
    ScheduleType_ATTEMPT, // 'soft'
    ScheduleType_FORCED,
    ScheduleType_SKIP,    // ????
};

class Action : public ISerializable {
public:
    Action()
    {
        m_type = ActionType_NONE;
    }

    virtual ~Action() {}

    virtual const string& getId()
    {
        return m_id;
    }

    virtual void setType(ActionType type)
    {
        m_type = type;
    }

    virtual ActionType getType()
    {
        return m_type;
    }

    virtual bool fromJson(const JValue& json) override
    {
        if (json.hasKey("id") && json["id"].isString()) {
            m_id = json["id"].asString();
        }
        return true;
    }

protected:
    string m_id;
    ActionType m_type;

};

class ActionInstall : public Action {
public:
    ActionInstall();
    virtual ~ActionInstall();

    ScheduleType getDownloadSchedule()
    {
        return m_download;
    }

    ScheduleType getUpdateSchedule()
    {
        return m_update;
    }

    bool isMaintenanceWindowAvailable()
    {
        return m_maintenanceWindow == "available";
    }

    const string& getHistoryStatus()
    {
        return m_historyStatus;
    }

    list<Chunk>& getChunks()
    {
        return m_chunks;
    }

    virtual bool fromJson(const JValue& json) override;

private:
    static string toString(enum ScheduleType& type);
    static ScheduleType toEnum(const string& type);

    ScheduleType m_download;
    ScheduleType m_update;
    string m_maintenanceWindow;
    string m_historyStatus;
    list<string> m_historyMessages;
    list<Chunk> m_chunks;
};

#endif /* CORE_ACTION_H_ */
