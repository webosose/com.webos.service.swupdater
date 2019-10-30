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
#include "updater/AbsUpdater.h"
#include "util/JValueUtil.h"
#include "util/Util.h"

// TODO change to /media/internal/downloads and delete downloaded files.
const string ArtifactLeaf::DIRNAME = "/home/root/";

ArtifactLeaf::ArtifactLeaf()
    : m_total(0)
    , m_curSize(0)
    , m_prevSize(0)
{
    setClassName("ArtifactLeaf");
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
        if (m_listener)
            m_listener->onChangedStatus(this);
        m_prevSize = m_curSize;
    }
}

void ArtifactLeaf::onCompletedDownload(HttpFile* call)
{
    Logger::info(getClassName(), m_fileName, __FUNCTION__);
    m_curSize = call->getFilesize();

    if (m_listener)
        m_listener->onCompletedDownload(this);
}

void ArtifactLeaf::onFailedDownload(HttpFile* call)
{
    Logger::error(getClassName(), m_fileName, __FUNCTION__);

    if (m_listener)
        m_listener->onFailedDownload(this);
}

void ArtifactLeaf::onInstallSubscription(pbnjson::JValue subscriptionPayload)
{
    string state;
    if (!JValueUtil::getValue(subscriptionPayload, "details", "state", state))
        return;

    LS2Handler::writeBLog("Return", "/install - " + state, subscriptionPayload);

    if (state == "installed") {
        getCall().cancel();
        if (m_listener)
            m_listener->onCompletedInstall(this);
    } else if (state == "install failed") {
        Logger::error(getClassName(), m_fileName, "Failed to install artifact");
        getCall().cancel();
        if (m_listener)
            m_listener->onFailedInstall(this);
    }
}

bool ArtifactLeaf::startDownload()
{
    Logger::debug(getClassName(), __FUNCTION__);

    m_httpFile = make_shared<HttpFile>();
    m_httpFile->open(MethodType_GET, m_url);
    m_httpFile->setFilename(getDownloadName());
    m_httpFile->setListener(this);
    // TODO return errorCode
    return m_httpFile->send();
}

bool ArtifactLeaf::pauseDownload()
{
    Logger::debug(getClassName(), __FUNCTION__);

    m_httpFile = nullptr;
    return true;
}

bool ArtifactLeaf::resumeDownload()
{
    Logger::debug(getClassName(), __FUNCTION__);

    m_httpFile = make_shared<HttpFile>();
    m_httpFile->open(MethodType_GET, m_url);
    m_httpFile->setFilename(getDownloadName());
    m_httpFile->setListener(this);
    // TODO return errorCode
    return m_httpFile->send();
}

bool ArtifactLeaf::cancelDownload()
{
    Logger::debug(getClassName(), __FUNCTION__);

    m_httpFile = nullptr;
    if (Util::removeFile(getDownloadName())) {
        m_curSize = 0;
        m_prevSize = 0;
    }
    return true;
}

bool ArtifactLeaf::startInstall()
{
    Logger::debug(getClassName(), __FUNCTION__);

    // Wait for this deployment action's status to be "install" and posting "getStatus".
    // Otherwise, "install" status comes after "onCompletedInstall" or "onFailedInstall".
    return Util::async([=] {
        if (Util::sha1(getDownloadName()) != m_sha1) {
            Logger::warning(getClassName(), m_fileName, "Sha1 verification failed");
            if (m_listener)
                m_listener->onFailedInstall(this);
            return true;
        }

        if (getFileExtension() == "ipk") {
            string installer = JValueUtil::getMeta(m_metadata, "installer");
            if (installer.empty() || installer == "appInstallService") {
                // TODO: Following is temp code for demo. we need to find better way
                string command = "opkg remove " + getIpkName();
                system(command.c_str());
                AppInstaller::getInstance().install(getIpkName(), getDownloadName(), this);
                return true;
            } else if (installer == "opkg") {
                AbsUpdaterFactory::getInstance().setReadWriteMode();
                string command = "opkg install --force-reinstall --force-downgrade " + getDownloadName();
                if (system(command.c_str()) == 0) {
                    if (m_listener)
                        m_listener->onCompletedInstall(this);
                } else {
                    if (m_listener)
                        m_listener->onFailedInstall(this);
                }
                return true;
            }
        } else if (getFileExtension() == "delta") {
            if (AbsUpdaterFactory::getInstance().deploy(getDownloadName())) {
                AbsUpdaterFactory::getInstance().printDebug();
                if (m_listener)
                    m_listener->onCompletedInstall(this);
            } else {
                if (m_listener)
                    m_listener->onFailedInstall(this);
            }
            return true;
        }

        Logger::warning(getClassName(), m_fileName, "Not supported file extension");
        if (m_listener)
            m_listener->onCompletedInstall(this);
        return true;
    }, 50);
}

bool ArtifactLeaf::cancelInstall()
{
    Logger::debug(getClassName(), __FUNCTION__);

    return AbsUpdaterFactory::getInstance().undeploy();
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
    Composite::toJson(json);

    json.put("filename", m_fileName);
    json.put("total", m_total);
    json.put("size", m_curSize);
    return true;
}
