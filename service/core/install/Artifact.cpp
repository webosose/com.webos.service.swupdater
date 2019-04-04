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

#include "core/install/Artifact.h"
#include "util/JValueUtil.h"
#include "PolicyManager.h"

const string Artifact::DIRNAME = "/home/root/";

Artifact::Artifact(const JValue& json)
    : m_curSize(0)
    , m_prevSize(0)
    , m_download("Artifact-download")
    , m_update("Artifact-update")
    , m_updateInProgress(false)
{
    setClassName("Artifact");
    fromJson(json);
}

Artifact::~Artifact()
{
    m_httpFile = nullptr;
}

void Artifact::onStartedDownload(HttpFile* call)
{
    Logger::info(getClassName(), "Start downloading - " + call->getFilename());
    m_curSize = call->getFilesize();
    PolicyManager::getInstance().onChangeStatus();
}

void Artifact::onProgressDownload(HttpFile* call)
{
    m_curSize = call->getFilesize();

    // To avoid many subscription issues.
    if ((m_curSize - m_prevSize) > (1024 * 512)) {
        Logger::verbose(getClassName(), "Progress downloading - " + call->getFilename());
        PolicyManager::getInstance().onChangeStatus();
        m_prevSize = m_curSize;
    }
}

void Artifact::onCompletedDownload(HttpFile* call)
{
    if (m_download.canComplete() != TransitionType_Allowed) {
        return;
    }

    Logger::info(getClassName(), "Complete downloading - " + call->getFilename());
    m_curSize = call->getFilesize();

    if (!m_download.complete()) {
        Logger::warning(getClassName(), "Invalid transition");
    }
}

void Artifact::onFailedDownload(HttpFile* call)
{
    if (m_download.canFail() != TransitionType_Allowed) {
        return;
    }

    Logger::warning(getClassName(), "Fail downloading - " + call->getFilename());
    m_curSize = 0;
    m_download.fail();
}

bool Artifact::prepareDownload()
{
    if (m_download.canPrepare() != TransitionType_Allowed) {
        return false;
    }

    m_httpFile = make_shared<HttpFile>();
    m_httpFile->open(MethodType_GET, m_url);
    m_httpFile->setFilename(getFullName());
    m_httpFile->setListener(this);

    return m_download.prepare();
}

bool Artifact::startDownload()
{
    if (m_download.canStart() != TransitionType_Allowed) {
        return false;
    }

    if (!m_httpFile->send()) {
        m_download.fail();
        return false;
    }
    return m_download.start();
}


void Artifact::onInstallSubscription(pbnjson::JValue subscriptionPayload)
{
    string state;
    if (!JValueUtil::getValue(subscriptionPayload, "details", "state", state))
        return;

    if (state == "installed") {
        getCall().cancel();
        m_update.complete();
    } else if (state == "install failed") {
        getCall().cancel();
        m_update.fail();
    }
}

bool Artifact::prepareUpdate()
{
    enum TransitionType type = m_update.canPrepare();
    if (type != TransitionType_Allowed) {
        return State::writeCommonLog(m_update, type, getClassName(), "prepare");
    }
    return m_update.prepare();
}

bool Artifact::startUpdate()
{
    enum TransitionType type = m_update.canStart();
    if (type == TransitionType_NotAllowed) {
        Logger::verbose(getClassName(), "Transition is not allowed");
        return false;
    }
    if (m_updateInProgress) {
        Logger::verbose(getClassName(), "Update is in progress");
        return true;
    } else if (m_download.getState() == StateType_COMPLETED) {
        m_updateInProgress = true;
        if (getFileExtension() == "ipk") {
            AppInstaller::getInstance().install(getIpkName(), getFullName(), this);
        } else {
            Logger::info(getClassName(), "Unsupported file type. Just completed");
            return m_update.complete();
        }
    } else {
        cout << m_download.getStateStr() << endl;
        Logger::verbose(getClassName(), "Need to wait more time");
    }
    return m_update.start();
}

bool Artifact::fromJson(const JValue& json)
{
    ISerializable::fromJson(json);

    JValueUtil::getValue(json, "size", m_total);
    JValueUtil::getValue(json, "filename", m_fileName);
    JValueUtil::getValue(json, "hashes", "sha1", m_sha1);
    JValueUtil::getValue(json, "hashes", "md5", m_md5);

    JValueUtil::getValue(json, "_links", "md5sum", "href", m_md5sum);
    JValueUtil::getValue(json, "_links", "download", "href", m_url);

    if (m_md5sum.empty()) {
        JValueUtil::getValue(json, "_links", "md5sum-http", "href", m_md5sum);
        JValueUtil::getValue(json, "_links", "download-http", "href", m_url);
    }
    return true;
}

bool Artifact::toJson(JValue& json)
{
    json.put("filename", m_fileName);
    json.put("total", m_total);
    json.put("size", m_curSize);
    json.put("download", m_download.getStateStr());
    json.put("update", m_update.getStateStr());
    return true;
}
