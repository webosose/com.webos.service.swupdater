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
#include "ls2/AppInstaller.h"
#include "PolicyManager.h"
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
    m_downloadState.setName("download");
    m_downloadState.setName("update");
}

SoftwareModule::~SoftwareModule()
{
}

bool SoftwareModule::ready(bool download)
{
    if (download) {
        for (auto it = m_artifacts.begin(); it != m_artifacts.end(); ++it) {
            if (!it->ready() || !m_downloadState.ready()) {
                return false;
            }
        }
    }
    return m_updateState.ready();
}

bool SoftwareModule::start(bool download)
{
    if (download) {
        // start to download all artifacts
        for (auto it = m_artifacts.begin(); it != m_artifacts.end(); ++it) {
            if (!it->start() || !m_downloadState.start()) {
                return false;
            }
        }
    }

    // All files are ready. Then call state::start() with
    return true;
}

bool SoftwareModule::pause(bool download)
{
    return false;
}

bool SoftwareModule::resume(bool download)
{
    return false;
}

bool SoftwareModule::cancel(bool download)
{
    return false;
}

void SoftwareModule::onDownloadStateChanged(State *installer, enum StateType prev, enum StateType cur)
{
    if (cur == StateType_FAILED) {
        m_updateState.fail();
    }

    for (auto it = m_artifacts.begin(); it != m_artifacts.end(); ++it) {
        if (it->getState() != cur) {
            return;
        }
    }

    State::transition(m_downloadState, cur);

    if (m_downloadState.getState() == StateType_COMPLETED && m_updateState.getState() == StateType_READY) {
        startUpdate();
    }
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
            m_artifacts.back().setCallback( // @suppress("Invalid arguments")
                std::bind(&SoftwareModule::onDownloadStateChanged,
                          this,
                          std::placeholders::_1,
                          std::placeholders::_2,
                          std::placeholders::_3
                )
            );
        }
    }
    return true;
}

bool SoftwareModule::toJson(JValue& json)
{
    json.put("type", toString(m_type));
    json.put("name", m_name);
    json.put("version", m_version);
    json.put("download", State::toString(m_downloadState.getState()));
    json.put("update", State::toString(m_updateState.getState()));

    JValue artifacts = pbnjson::Array();
    for (auto it = m_artifacts.begin(); it != m_artifacts.end(); ++it) {
        JValue artifact = pbnjson::Object();
        it->toJson(artifact);
        artifacts.append(artifact);
    }
    json.put("artifacts", artifacts);
    return true;
}
