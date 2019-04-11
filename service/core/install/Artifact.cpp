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

Artifact::Artifact()
    : m_total(0)
    , m_curSize(0)
    , m_prevSize(0)
    , m_updateInProgress(false)
{
    setClassName("Artifact");
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
    if (m_status.canComplete() != TransitionType_Allowed) {
        return;
    }

    m_curSize = call->getFilesize();

    if (m_status.getStatus() != StatusType_RUNNING) {
        return;
    }


    if (getFileExtension() == "ipk") {
        AppInstaller::getInstance().install(getIpkName(), getFullName(), this);
    } else {
        Logger::warning(getClassName(), m_fileName, "Not supported file extension");
        m_status.complete();
    }
}

void Artifact::onFailedDownload(HttpFile* call)
{
    Logger::error(getClassName(), m_fileName, __FUNCTION__);
    if (m_status.canFail() != TransitionType_Allowed) {
        return;
    }

    m_curSize = 0;
    m_status.fail();
}

void Artifact::onInstallSubscription(pbnjson::JValue subscriptionPayload)
{
    string state;
    if (!JValueUtil::getValue(subscriptionPayload, "details", "state", state))
        return;

    LS2Handler::writeBLog("Return", "/install", subscriptionPayload);

    if (state == "installed") {
        getCall().cancel();
        m_status.complete();
    } else if (state == "install failed") {
        Logger::error(getClassName(), m_fileName, "Failed to install artifact");
        getCall().cancel();
        m_status.fail();
    }
}

bool Artifact::prepare()
{
    if (!Leaf::prepare())
        return false;
    m_httpFile = make_shared<HttpFile>();
    m_httpFile->open(MethodType_GET, m_url);
    m_httpFile->setFilename(getFullName());
    m_httpFile->setListener(this);

    return true;
}

bool Artifact::install()
{
    if (!Leaf::install())
        return false;
    if (!m_httpFile->send()) {
        m_status.fail();
        return false;
    }
    return true;
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
    Component::toJson(json);

    json.put("filename", m_fileName);
    json.put("total", m_total);
    json.put("size", m_curSize);
    return true;
}
