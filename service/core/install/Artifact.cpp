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
    Logger::info(getClassName(), m_fileName, __FUNCTION__);
    m_curSize = call->getFilesize();
}

void Artifact::onProgressDownload(HttpFile* call)
{
    m_curSize = call->getFilesize();

    // To avoid many subscription issues.
    if ((m_curSize - m_prevSize) > (1024 * 512)) {
        Logger::debug(getClassName(), m_fileName, std::string(__FUNCTION__) + " (" + to_string(m_curSize) + "/" + to_string(m_total) + ")");
        // TODO Need to change progress handler
        PolicyManager::getInstance().onChangeStatus();
        m_prevSize = m_curSize;
    }
}

void Artifact::onCompletedDownload(HttpFile* call)
{
    Logger::info(getClassName(), m_fileName, __FUNCTION__);
    if (m_download.canComplete() != TransitionType_Allowed) {
        return;
    }

    m_curSize = call->getFilesize();

    if (!m_download.complete()) {
        Logger::error(getClassName(), m_fileName, "Failed to complete download");
    }
}

void Artifact::onFailedDownload(HttpFile* call)
{
    Logger::error(getClassName(), m_fileName, __FUNCTION__);
    if (m_download.canFail() != TransitionType_Allowed) {
        return;
    }

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

    LS2Handler::writeBLog("Return", "/install", subscriptionPayload);

    if (state == "installed") {
        getCall().cancel();
        m_update.complete();
    } else if (state == "install failed") {
        Logger::error(getClassName(), m_fileName, "Failed to install artifact");
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
        Logger::error(getClassName(), m_fileName, "Translation is not allowed");
        return false;
    }
    if (m_updateInProgress) {
        Logger::debug(getClassName(), m_fileName, "Update is in progress");
        return true;
    } else if (m_download.getState() == StateType_COMPLETED) {
        m_updateInProgress = true;
        if (getFileExtension() == "ipk") {
            AppInstaller::getInstance().install(getIpkName(), getFullName(), this);
        } else {
            Logger::warning(getClassName(), m_fileName, "Not supported file extension");
            return m_update.complete();
        }
    } else {
        Logger::debug(getClassName(), m_fileName, "Download is not completed");
    }
    return m_update.wait();
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
