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
#include <core/install/ApplicationSoftwareModule.h>

string SoftwareModule::toString(enum SoftwareModuleType& type)
{
    switch(type){
    case SoftwareModuleType_Unknown:
        return "unknown";
    case SoftwareModuleType_Application:
        return "bApp";
    default:
        break;
    }
    return "unknown";
}

SoftwareModuleType SoftwareModule::toEnum(const string& type)
{
    if (type == "unknown") {
        return SoftwareModuleType_Unknown;
    } else if (type == "bApp") {
        return SoftwareModuleType_Application;
    }
    return SoftwareModuleType_Unknown;
}

shared_ptr<SoftwareModule> SoftwareModule::createSoftwareModule(JValue& json)
{
    shared_ptr<SoftwareModule> softwareModule = nullptr;

    if (!json.hasKey("part"))
        return nullptr;

    if (SoftwareModule::toEnum(json["part"].asString()) == SoftwareModuleType_Application) {
        softwareModule = make_shared<ApplicationSoftwareModule>();
        softwareModule->fromJson(json);
    }

    return softwareModule;
}

SoftwareModule::SoftwareModule()
    : m_type(SoftwareModuleType_Unknown)
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
    if (json.hasKey("artifacts") && json["artifacts"].isArray()) {
        for (JValue artifact : json["artifacts"].items()) {
            m_artifacts.emplace_back(artifact);
        }
    }
    return true;
}
