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

#include "core/install/impl/ArtifactLeaf.h"

#include "PolicyManager.h"
#include "ostree/OSTree.h"
#include "util/JValueUtil.h"

const string ArtifactLeaf::DIRNAME = "/home/root/";

ArtifactLeaf::ArtifactLeaf()
    : m_total(0)
    , m_curSize(0)
    , m_prevSize(0)
{
    setClassName("ArtifactLeaf");
    m_status.setName("ArtifactLeaf");
}

ArtifactLeaf::~ArtifactLeaf()
{
    m_httpFile = nullptr;
}

void ArtifactLeaf::onStartedDownload(HttpFile* call)
{
    Logger::info(getClassName(), m_fileName, __FUNCTION__);
    m_curSize = call->getFilesize();
}

void ArtifactLeaf::onProgressDownload(HttpFile* call)
{
    m_curSize = call->getFilesize();

    // To avoid many subscription issues.
    if ((m_curSize - m_prevSize) > (1024 * 512)) {
        Logger::debug(getClassName(), m_fileName, std::string(__FUNCTION__) + " (" + to_string(m_curSize) + "/" + to_string(m_total) + ")");
        // TODO Need to change progress handler
        PolicyManager::getInstance().onRequestProgressUpdate();
        m_prevSize = m_curSize;
    }
}

void ArtifactLeaf::onCompletedDownload(HttpFile* call)
{
    Logger::info(getClassName(), m_fileName, __FUNCTION__);
    m_curSize = call->getFilesize();

    if (m_status.getStatus() != StatusType_RUNNING) {
        return;
    }

    if (getFileExtension() == "ipk") {
        string installer = JValueUtil::getMeta(m_metadata, "installer");
        if (installer.empty() || installer == "appInstallService") {
            // TODO: Following is temp code for demo. we need to find better way
            string command = "opkg remove " + getIpkName();
            system(command.c_str());
            AppInstaller::getInstance().install(getIpkName(), getFullName(), this);
            return;
        } else if (installer == "opkg") {
            string command = "opkg install --force-reinstall --force-downgrade " + getFullName();
            if (system(command.c_str()) == 0)
                m_status.complete();
            else
                m_status.fail();
            return;
        }
    } else if (getFileExtension() == "delta") {
        if (OSTree::getInstance().deployDelta(getFullName())) {
            OSTree::getInstance().printDebug();
            m_status.complete();
        } else {
            m_status.fail();
        }
        return;
    }

    Logger::warning(getClassName(), m_fileName, "Not supported file extension");
    m_status.complete();
}

void ArtifactLeaf::onFailedDownload(HttpFile* call)
{
    Logger::error(getClassName(), m_fileName, __FUNCTION__);
    m_status.fail();
}

void ArtifactLeaf::onInstallSubscription(pbnjson::JValue subscriptionPayload)
{
    string state;
    if (!JValueUtil::getValue(subscriptionPayload, "details", "state", state))
        return;

    LS2Handler::writeBLog("Return", "/install - " + state, subscriptionPayload);

    if (state == "installed") {
        getCall().cancel();
        m_status.complete();
    } else if (state == "install failed") {
        Logger::error(getClassName(), m_fileName, "Failed to install artifact");
        getCall().cancel();
        m_status.fail();
    }
}

bool ArtifactLeaf::prepare()
{
    if (!Leaf::prepare())
        return false;
    m_httpFile = make_shared<HttpFile>();
    m_httpFile->open(MethodType_GET, m_url);
    m_httpFile->setFilename(getFullName());
    m_httpFile->setListener(this);
    return true;
}

bool ArtifactLeaf::start()
{
    if (!Leaf::start())
        return false;
    if (!m_httpFile->send()) {
        m_status.fail();
        return false;
    }
    return true;
}

bool ArtifactLeaf::pause()
{
    if (!Leaf::pause())
        return false;
    // TODO download should be paused
    return true;
}

bool ArtifactLeaf::resume()
{
    if (!Leaf::resume())
        return false;
    // TODO download should be resumed
    return true;
}

bool ArtifactLeaf::cancel()
{
    if (!Leaf::cancel())
        return false;
    // TODO download should be canceled
    return true;
}

bool ArtifactLeaf::fromJson(const JValue& json)
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

bool ArtifactLeaf::toJson(JValue& json)
{
    Component::toJson(json);

    json.put("filename", m_fileName);
    json.put("total", m_total);
    json.put("size", m_curSize);
    return true;
}
