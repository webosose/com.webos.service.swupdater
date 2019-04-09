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
#include "ls2/AppInstaller.h"
#include "PolicyManager.h"
#include "util/JValueUtil.h"

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

SoftwareModule::SoftwareModule(JValue& json)
    : m_type(SoftwareModuleType_Unknown)
    , m_name("")
    , m_version("")
    , m_download("SoftwareModule-download")
    , m_update("SoftwareModule-update")
    , m_curDownload(0)
    , m_curUpdate(0)
{
    setClassName("SoftwareModule");
    fromJson(json);
}

SoftwareModule::~SoftwareModule()
{
    removeCallback();
}

void SoftwareModule::onDownloadStateChanged(enum StateType prev, enum StateType cur)
{
    switch (cur) {
    case StateType_NONE:
        break;

    case StateType_READY:
        break;

    case StateType_WAITING:
        break;

    case StateType_RUNNING:
        break;

    case StateType_PAUSED:
        break;

    case StateType_CANCELED:
        m_download.cancel();
        break;

    case StateType_COMPLETED:
        if (m_curDownload == m_artifacts.size() -1) {
            m_download.complete();
            break;
        }
        m_curDownload++;
        m_artifacts[m_curDownload].startDownload();
        break;

    case StateType_FAILED:
        m_download.fail();
        break;
    }

    // All artifacts are downloaded
    if (m_download.getState() == StateType_COMPLETED && m_update.getState() == StateType_WAITING) {
        m_artifacts[m_curUpdate].startUpdate();
    }
}

bool SoftwareModule::prepareDownload()
{
    enum TransitionType type = m_download.canPrepare();
    if (type != TransitionType_Allowed) {
        return State::writeCommonLog(m_download, type, getClassName(), "prepare");
    }
    for (auto it = m_artifacts.begin(); it != m_artifacts.end(); ++it) {
        if (!it->prepareDownload()) {
            m_download.fail();
            return false;
        }
    }
    return m_download.prepare();
}

bool SoftwareModule::startDownload()
{
    enum TransitionType type = m_download.canStart();
    if (type != TransitionType_Allowed) {
        return State::writeCommonLog(m_download, type, getClassName(), "start");
    }

    m_curDownload = 0;
    if (!m_artifacts[m_curDownload].startDownload()) {
        Logger::verbose(getClassName(), m_artifacts.begin()->getFileName(), "Start download");
        return false;
    }
    return m_download.start();
}

void SoftwareModule::onUpdateStateChanged(enum StateType prev, enum StateType cur)
{
    switch (cur) {
     case StateType_NONE:
         break;

     case StateType_READY:
         break;

     case StateType_WAITING:
         break;

     case StateType_RUNNING:
         break;

     case StateType_PAUSED:
         break;

     case StateType_CANCELED:
         m_update.canCancel();
         break;

     case StateType_COMPLETED:
         if (m_curUpdate == m_artifacts.size() -1) {
             m_update.complete();
             break;
         }
         m_curUpdate++;
         m_artifacts[m_curUpdate].startUpdate();
         break;

     case StateType_FAILED:
         m_update.fail();
         break;
     }
}

bool SoftwareModule::prepareUpdate()
{
    enum TransitionType type = m_update.canPrepare();
    if (type != TransitionType_Allowed) {
        return State::writeCommonLog(m_update, type, getClassName(), "prepare");
    }
    for (auto it = m_artifacts.begin(); it != m_artifacts.end(); ++it) {
        if (!it->prepareUpdate()) {
            m_update.fail();
            return false;
        }
    }
    return m_update.prepare();
}

bool SoftwareModule::startUpdate()
{
    enum TransitionType type = m_update.canStart();
    if (type != TransitionType_Allowed) {
        return State::writeCommonLog(m_update, type, getClassName(), "start");
    }

    m_curUpdate = 0;
    if (m_download.getState() == StateType_COMPLETED) {
        m_artifacts[m_curUpdate].startUpdate();
        return m_update.start();
    } else {
        return m_update.wait();
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
        }
    }

    addCallback();
    return true;
}

bool SoftwareModule::toJson(JValue& json)
{
    json.put("type", toString(m_type));
    json.put("name", m_name);
    json.put("version", m_version);
    json.put("download", m_download.getStateStr());
    json.put("update", m_update.getStateStr());

    JValue artifacts = pbnjson::Array();
    for (auto it = m_artifacts.begin(); it != m_artifacts.end(); ++it) {
        JValue artifact = pbnjson::Object();
        it->toJson(artifact);
        artifacts.append(artifact);
    }
    json.put("artifacts", artifacts);
    return true;
}

void SoftwareModule::addCallback()
{
    for (auto it = m_artifacts.begin(); it != m_artifacts.end(); ++it) {
        it->getDownload().setCallback( // @suppress("Invalid arguments")
            std::bind(&SoftwareModule::onDownloadStateChanged,
                      this,
                      std::placeholders::_1,
                      std::placeholders::_2
            )
        );
        it->getUpdate().setCallback( // @suppress("Invalid arguments")
            std::bind(&SoftwareModule::onUpdateStateChanged,
                      this,
                      std::placeholders::_1,
                      std::placeholders::_2
            )
        );
    }
}

void SoftwareModule::removeCallback()
{
    for (auto it = m_artifacts.begin(); it != m_artifacts.end(); ++it) {
        it->getDownload().setCallback(nullptr);
        it->getUpdate().setCallback(nullptr);
    }
}
