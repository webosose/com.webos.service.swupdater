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

Artifact::Artifact(const JValue& json)
{
    setClassName("Artifact");
    setName("Artifact");
    fromJson(json);
}

Artifact::~Artifact()
{
    m_httpCall = nullptr;
}

bool Artifact::ready()
{
    m_httpCall = make_shared<HttpCall>(MethodType_GET, m_download);
    m_httpCall->setFilename(m_fullname);
    m_httpCall->setListener(this);
    return State::ready();
}

bool Artifact::start()
{
    m_httpCall->download();
    return State::start();
}

void Artifact::onStartedDownload(HttpCall* call)
{
    Logger::info(getClassName(), "Start downloading - " + call->getFilename());
    m_size = call->getResponseSize();
    PolicyManager::getInstance().onChangeStatus();
}

void Artifact::onProgressDownload(HttpCall* call)
{
    Logger::info(getClassName(), "Progress downloading - " + call->getFilename());
    m_size = call->getResponseSize();
    PolicyManager::getInstance().onChangeStatus();
}

void Artifact::onCompletedDownload(HttpCall* call)
{
    Logger::info(getClassName(), "Complete downloading - " + call->getFilename());
    m_size = call->getResponseSize();
    complete();
}

void Artifact::onFailedDownload(HttpCall* call)
{
    Logger::info(getClassName(), "Fail downloading - " + call->getFilename());
    m_size = 0;
    fail();
}

bool Artifact::fromJson(const JValue& json)
{
    ISerializable::fromJson(json);

    JValueUtil::getValue(json, "size", m_total);
    JValueUtil::getValue(json, "filename", m_filename);
    JValueUtil::getValue(json, "hashes", "sha1", m_sha1);
    JValueUtil::getValue(json, "hashes", "md5", m_md5);

    JValueUtil::getValue(json, "_links", "md5sum", "href", m_md5sum);
    JValueUtil::getValue(json, "_links", "download", "href", m_download);

    if (m_md5sum.empty()) {
        JValueUtil::getValue(json, "_links", "md5sum-http", "href", m_md5sum);
        JValueUtil::getValue(json, "_links", "download-http", "href", m_download);
    }

    m_fullname = "/home/root/" + m_filename;
    return true;
}

bool Artifact::toJson(JValue& json)
{
    json.put("filename", m_filename);
    json.put("total", m_total);
    json.put("size", m_size);
    json.put("download", State::toString(getState()));
    return true;
}
