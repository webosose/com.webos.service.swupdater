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

#include "core/install/SoftwareModule.h"

#include "core/install/AppSoftwareModule.h"
#include "core/install/OSSoftwareModule.h"
#include "util/JValueUtil.h"

shared_ptr<SoftwareModule> SoftwareModule::createSoftwareModule(JValue& json)
{
    shared_ptr<SoftwareModule> softwareModule = nullptr;

    if (!json.hasKey("part"))
        return nullptr;

    if (SoftwareModule::toEnum(json["part"].asString()) == SoftwareModuleType_Application) {
        softwareModule = make_shared<AppSoftwareModule>();
        softwareModule->fromJson(json);
    } else if (SoftwareModule::toEnum(json["part"].asString()) == SoftwareModuleType_OS) {
        softwareModule = make_shared<OSSoftwareModule>();
        softwareModule->fromJson(json);
    }

    return softwareModule;
}

string SoftwareModule::toString(enum SoftwareModuleType& type)
{
    switch(type){
    case SoftwareModuleType_Unknown:
        return "unknown";

    case SoftwareModuleType_Application:
        return "application";

    case SoftwareModuleType_OS:
        return "os";

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

SoftwareModule::SoftwareModule()
    : m_type(SoftwareModuleType_Unknown)
    , m_name("")
    , m_version("")
{
    setClassName("SoftwareModule");
}

SoftwareModule::~SoftwareModule()
{
}

bool SoftwareModule::onReadyDownloading()
{
    for (auto it = m_artifacts.begin(); it != m_artifacts.end(); ++it) {
        it->readyDownloading();
    }
    return true;
}

bool SoftwareModule::onStartDownloading()
{
    for (auto it = m_artifacts.begin(); it != m_artifacts.end(); ++it) {
        it->startDownloading();
    }
    return true;
}

bool SoftwareModule::onReadyInstallation()
{
    for (auto it = m_artifacts.begin(); it != m_artifacts.end(); ++it) {
        it->readyInstallation();
    }
    return true;
}

bool SoftwareModule::onStartInstallation()
{
    for (auto it = m_artifacts.begin(); it != m_artifacts.end(); ++it) {
        it->setListener(this);
        it->startInstallation();
    }
    return true;
}

bool SoftwareModule::fromJson(const JValue& json)
{
    ISerializable::fromJson(json);

    string part;
    JValueUtil::getValue(json, "part", part);
    if (!part.empty()) {
        m_type = toEnum(part);
    }
    JValueUtil::getValue(json, "name", m_name);
    JValueUtil::getValue(json, "version", m_version);
    if (json.hasKey("artifacts") && json["artifacts"].isArray()) {
        for (JValue artifact : json["artifacts"].items()) {
            m_artifacts.emplace_back(artifact);
            m_artifacts.back().setListener(this);
        }
    }
    return true;
}

bool SoftwareModule::toJson(JValue& json)
{
    json.put("type", toString(m_type));
    json.put("name", m_name);
    json.put("m_version", m_version);

    JValue artifacts = pbnjson::Array();
    for (auto it = m_artifacts.begin(); it != m_artifacts.end(); ++it) {
        JValue artifact = pbnjson::Object();
        it->toJson(artifact);
        artifacts.append(artifact);
    }
    json.put("artifacts", artifacts);
    return true;
}
