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

#include <core/install/Artifact.h>

Artifact::Artifact()
    : m_size(0)
{
}

Artifact::~Artifact()
{
}

bool Artifact::fromJson(const JValue& json)
{
    ISerializable::fromJson(json);
    if (json.hasKey("filename") && json["filename"].isString()) {
        m_filename = json["filename"].asString();
    }
    if (json.hasKey("size") && json["size"].isNumber()) {
        m_size = json["size"].asNumber<int>();
    }
    if (json.hasKey("hashes")) {
        JValue hashes = json["hashes"];
        if (hashes.hasKey("sha1") && hashes["sha1"].isString()) {
            m_sha1 = hashes["sha1"].asString();
        }
        if (hashes.hasKey("md5") && hashes["md5"].isString()) {
            m_md5 = hashes["md5"].asString();
        }
    }

    if (!json.hasKey("_links")) {
        return false;
    }
    JValue _links = json["_links"];
    if (_links.hasKey("download")) {
        JValue http = _links["download"];
        if (http.hasKey("href") && http["href"].isString()) {
            m_downloadHttps = http["href"].asString();
        }
    }
    if (_links.hasKey("download-http")) {
        JValue http = _links["download-http"];
        if (http.hasKey("href") && http["href"].isString()) {
            m_downloadHttp = http["href"].asString();
        }
    }
    if (_links.hasKey("md5sum")) {
        JValue http = _links["md5sum"];
        if (http.hasKey("href") && http["href"].isString()) {
            m_md5sumHttps = http["href"].asString();
        }
    }
    if (_links.hasKey("md5sum-http")) {
        JValue http = _links["md5sum-http"];
        if (http.hasKey("href") && http["href"].isString()) {
            m_md5sumHttp = http["href"].asString();
        }
    }
    return true;
}
