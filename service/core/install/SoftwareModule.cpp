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

#include <core/install/SoftwareModule.h>

string SoftwareModule::toString(enum ChunkType& type)
{
    switch(type){
    case ChunkType_Unknown:
        return "unknown";
    case ChunkType_Application:
        return "bApp";
    default:
        break;
    }
    return "unknown";
}

ChunkType SoftwareModule::toEnum(const string& type)
{
    if (type == "unknown") {
        return ChunkType_Unknown;
    } else if (type == "bApp") {
        return ChunkType_Application;
    }
    return ChunkType_Unknown;
}

SoftwareModule::SoftwareModule()
    : m_type(ChunkType_Unknown)
    , m_name("")
    , m_version("")
{
}

SoftwareModule::~SoftwareModule()
{
}

bool SoftwareModule::fromJson(const JValue& json)
{
    ISerializable::fromJson(json);
    if (json.hasKey("part") && json["part"].isString()) {
        m_type = toEnum(json["part"].asString());
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
