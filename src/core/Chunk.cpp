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

#include "Chunk.h"

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

bool Chunk::toJson(JValue& json)
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
        artifactObj.toJson(artifact);
        artifacts.append(artifact);
    }
    json.put("artifacts", artifacts);
    return true;
}
