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

Artifact::Artifact(const JValue& json)
{
    setClassName("Artifact");
    fromJson(json);
    download();
}

Artifact::~Artifact()
{
}

bool Artifact::download()
{
    m_httpCall = make_shared<HttpCall>(MethodType_GET, m_download);
    m_httpCall->setListener(this);
    m_httpCall->setFilename("/tmp/test.ipk");
    return m_httpCall->download();
}

void Artifact::onCompleteDownload(HttpCall& call)
{
    Logger::verbose(getClassName(), std::string(__FUNCTION__) + " is called");
}

bool Artifact::fromJson(const JValue& json)
{
    ISerializable::fromJson(json);

    JValueUtil::getValue(json, "size", m_size);
    JValueUtil::getValue(json, "filename", m_filename);
    JValueUtil::getValue(json, "hashes", "sha1", m_sha1);
    JValueUtil::getValue(json, "hashes", "md5", m_md5);

    JValueUtil::getValue(json, "_links", "md5sum", "href", m_md5sum);
    JValueUtil::getValue(json, "_links", "download", "href", m_download);

    if (m_md5sum.empty()) {
        JValueUtil::getValue(json, "_links", "md5sum-http", "href", m_md5sum);
        JValueUtil::getValue(json, "_links", "download-http", "href", m_download);
    }

    return true;
}
