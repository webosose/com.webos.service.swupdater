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

#include "Action.h"

ActionCancel::ActionCancel()
    : Action()
{
}

ActionCancel::~ActionCancel()
{
}

bool ActionCancel::fromJson(const JValue& json)
{
    if (json.hasKey("id") && json["id"].isString()) {
        m_id = json["id"].asString();
    }
    return true;
}

void ActionCancel::toDebugJson(JValue& json)
{
    json.put("id", m_id);
}

ActionInstall::ActionInstall()
    : Action()
    , m_downloadSchedule(ScheduleType_UNKNOWN)
    , m_updateSchedule(ScheduleType_UNKNOWN)
{
}

ActionInstall::~ActionInstall()
{
}

ScheduleType ActionInstall::getScheduleTypeEnum(const string& type)
{
    if (type == "attempt")
        return ScheduleType_ATTEMPT;
    if (type == "forced")
        return ScheduleType_FORCED;
    if (type == "skip")
        return ScheduleType_SKIP;

    return ScheduleType_UNKNOWN;
}

string ActionInstall::toString(enum ScheduleType type)
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

bool ActionInstall::fromJson(const JValue& json)
{
    if (json.hasKey("id") && json["id"].isString()) {
        m_id = json["id"].asString();
    }
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
            Chunk chunk;
            chunk.fromJson(chunkJson);
            m_chunks.push_back(chunk);
        }
    }
    if (deployment.hasKey("download") && deployment["download"].isString()) {
        string download = deployment["download"].asString();
        m_downloadSchedule = getScheduleTypeEnum(download);
    }
    if (deployment.hasKey("update") && deployment["update"].isString()) {
        string update = deployment["download"].asString();
        m_updateSchedule = getScheduleTypeEnum(update);
    }
    if (deployment.hasKey("maintenanceWindow") && deployment["maintenanceWindow"].isString()) {
        m_maintenanceWindow = deployment["maintenanceWindow"].asString();
    }
    return true;
}

void ActionInstall::toDebugJson(JValue& json)
{
    json.put("id", m_id);

    JValue actionHistory = Object();
    actionHistory.put("status", m_historyStatus);
    JValue actionHistoryMessages = Array();
    for (string message : m_historyMessages) {
        actionHistoryMessages.append(message);
    }
    actionHistory.put("messages", actionHistoryMessages);
    json.put("actionHistory", actionHistory);

    JValue deployment = Object();
    deployment.put("download", toString(m_downloadSchedule));
    deployment.put("update", toString(m_updateSchedule));
    deployment.put("maintenanceWindow", m_maintenanceWindow);
    JValue chunks = Array();
    for (Chunk chunkObj : m_chunks) {
        JValue chunk = Object();
        chunkObj.toDebugJson(chunk);
        chunks.append(chunk);
    }
    deployment.put("chunks", chunks);
    json.put("deployment", deployment);
}

Chunk::Chunk()
{
}

Chunk::~Chunk()
{
}

bool Chunk::fromJson(const JValue& json)
{
    if (json.hasKey("part") && json["part"].isString()) {
        m_part = json["part"].asString();
    }
    if (json.hasKey("name") && json["name"].isString()) {
        m_name = json["name"].asString();
    }
    if (json.hasKey("version") && json["version"].isString()) {
        m_version = json["version"].asString();
    }
    if (json.hasKey("metadata") && json["metadata"].isArray()) {
        for (JValue metadata : json["metadata"].items()) {
            string key = metadata["key"].asString();
            string value = metadata["value"].asString();
            m_metadata[key] = value;
        }
    }
    if (json.hasKey("artifacts") && json["artifacts"].isArray()) {
        for (JValue artifactJson : json["artifacts"].items()) {
            Artifact artifact;
            artifact.fromJson(artifactJson);
            m_artifacts.push_back(artifact);
        }
    }
    return true;
}

void Chunk::toDebugJson(JValue& json)
{
    json.put("part", m_part);
    json.put("name", m_name);
    json.put("version", m_version);

    JValue metadata = Array();
    for (auto& kv : m_metadata) {
        JValue metaItem = Object();
        metaItem.put("key", kv.first);
        metaItem.put("value", kv.second);
        metadata.append(metaItem);
    }
    json.put("metadata", metadata);

    JValue artifacts = Array();
    for (Artifact& artifactObj : m_artifacts) {
        JValue artifact = Object();
        artifactObj.toDebugJson(artifact);
        artifacts.append(artifact);
    }
    json.put("artifacts", artifacts);
}

Artifact::Artifact()
    : m_size(0)
{
}

Artifact::~Artifact()
{
}

bool Artifact::fromJson(const JValue& json)
{
    if (json.hasKey("filename") && json["filename"].isString()) {
        m_filename = json["filename"].asString();
    }
    if (json.hasKey("size") && json["size"].isNumber()) {
        m_size = json["size"].asNumber<int>();
    }
    if (json.hasKey("hashes")) {
        JValue hashes = json["hashes"];
        if (hashes.hasKey("sha1") && hashes["sha1"].isString()) {
            m_hashSha1 = hashes["sha1"].asString();
        }
        if (hashes.hasKey("md5") && hashes["md5"].isString()) {
            m_hashMd5 = hashes["md5"].asString();
        }
    }

    if (!json.hasKey("_links")) {
        return false;
    }
    JValue _links = json["_links"];
    if (_links.hasKey("download")) {
        JValue http = _links["download"];
        if (http.hasKey("href") && http["href"].isString()) {
            m_downloadHttps = http["href"].asString();
        }
    }
    if (_links.hasKey("md5sum")) {
        JValue http = _links["md5sum"];
        if (http.hasKey("href") && http["href"].isString()) {
            m_downloadMd5Https = http["href"].asString();
        }
    }
    if (_links.hasKey("download-http")) {
        JValue http = _links["download-http"];
        if (http.hasKey("href") && http["href"].isString()) {
            m_downloadHttp = http["href"].asString();
        }
    }
    if (_links.hasKey("md5sum-http")) {
        JValue http = _links["md5sum-http"];
        if (http.hasKey("href") && http["href"].isString()) {
            m_downloadMd5Http = http["href"].asString();
        }
    }
    return true;
}

void Artifact::toDebugJson(JValue& json)
{
    json.put("filename", m_filename);
    json.put("size", m_size);

    JValue hashes = Object();
    hashes.put("sha1", m_hashSha1);
    hashes.put("md5", m_hashMd5);
    json.put("hashes", hashes);

    JValue _links = Object();
    _links.put("download", m_downloadHttps);
    _links.put("md5sum", m_downloadMd5Https);
    _links.put("download-http", m_downloadHttp);
    _links.put("m5sum-http", m_downloadMd5Http);
    json.put("_links", _links);
}
