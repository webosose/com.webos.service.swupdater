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

using namespace pbnjson;
using namespace std;

enum ActionType {
    ActionType_NONE,
    ActionType_INSTALL,
    ActionType_CANCEL,
};

class Action {
public:
    Action() { };
    virtual ~Action() { };

    virtual const string& getId()
    {
        return m_id;
    }
    virtual ActionType getType() = 0;
    virtual bool fromJson(const JValue& json) = 0;
    virtual void toDebugJson(JValue& json) = 0;

protected:
    string m_id;
};

class Artifact {
public:
    Artifact();
    virtual ~Artifact();

    const string& getFilename()
    {
        return m_filename;
    }
    const string& getSha1Hash()
    {
        return m_hashSha1;
    }
    const string& getMd5Hash()
    {
        return m_hashMd5;
    }
    int getSize()
    {
        return m_size;
    }
    const string& getDownloadHttps()
    {
        return m_downloadHttps;
    }
    const string& getDownloadMd5Https()
    {
        return m_downloadMd5Https;
    }
    const string& getDownloadHttp()
    {
        return m_downloadHttp;
    }
    const string& getDownloadMd5Http()
    {
        return m_downloadMd5Http;
    }

    virtual bool fromJson(const JValue& json);
    virtual void toDebugJson(JValue& json);

private:
    string m_filename;
    string m_hashSha1;
    string m_hashMd5;
    int m_size;
    string m_downloadHttps;
    string m_downloadMd5Https;
    string m_downloadHttp;
    string m_downloadMd5Http;
};

class Chunk {
public:
    Chunk();
    virtual ~Chunk();

    const string& getPart()
    {
        return m_part;
    }
    const string& getName()
    {
        return m_name;
    }
    const string& getVersion()
    {
        return m_version;
    }
    const list<Artifact>& getArtifacts()
    {
        return m_artifacts;
    }

    virtual bool fromJson(const JValue& json);
    virtual void toDebugJson(JValue& json);

private:
    string m_part;
    string m_name;
    string m_version;
    list<Artifact> m_artifacts;
    map<string, string> m_metadata;
};

enum ScheduleType {
    ScheduleType_SKIP,
    ScheduleType_ATTEMPT,
    ScheduleType_FORCED,
    ScheduleType_UNKNOWN,
};

class ActionInstall : public Action {
public:
    ActionInstall();
    virtual ~ActionInstall();

    ScheduleType getDownloadSchedule()
    {
        return m_downloadSchedule;
    }
    ScheduleType getUpdateSchedule()
    {
        return m_updateSchedule;
    }
    bool isMaintenanceWindowAvailable()
    {
        return m_maintenanceWindow == "available";
    }
    const string& getHistoryStatus()
    {
        return m_historyStatus;
    }
    const list<Chunk>& getChunks()
    {
        return m_chunks;
    }

    virtual ActionType getType()
    {
        return ActionType_INSTALL;
    }
    virtual bool fromJson(const JValue& json);
    virtual void toDebugJson(JValue& json);

private:
    static string toString(enum ScheduleType type);
    static ScheduleType getScheduleTypeEnum(const string& type);

    ScheduleType m_downloadSchedule;
    ScheduleType m_updateSchedule;
    string m_maintenanceWindow;
    string m_historyStatus;
    list<string> m_historyMessages;
    list<Chunk> m_chunks;
};

class ActionCancel : public Action {
public:
    ActionCancel();
    virtual ~ActionCancel();

    virtual ActionType getType()
    {
        return ActionType_CANCEL;
    }
    virtual bool fromJson(const JValue& json);
    virtual void toDebugJson(JValue& json);
};

#endif /* CORE_ACTION_H_ */
