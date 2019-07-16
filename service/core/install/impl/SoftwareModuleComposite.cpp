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

#include "core/install/impl/SoftwareModuleComposite.h"

#include "PolicyManager.h"
#include "ls2/AppInstaller.h"
#include "util/JValueUtil.h"
#include "util/Logger.h"

string SoftwareModuleComposite::toString(enum SoftwareModuleType& type)
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

SoftwareModuleType SoftwareModuleComposite::toEnum(const string& type)
{
    if (type == "unknown") {
        return SoftwareModuleType_Unknown;
    } else if (type == "bApp") {
        return SoftwareModuleType_Application;
    } else if (type == "os") {
        return SoftwareModuleType_OS;
    }

    return SoftwareModuleType_Unknown;
}

SoftwareModuleComposite::SoftwareModuleComposite()
    : m_type(SoftwareModuleType_Unknown)
    , m_name("")
    , m_version("")
{
    setClassName("SoftwareModuleComposite");
    m_status.setName("SoftwareModuleComposite");
}

SoftwareModuleComposite::~SoftwareModuleComposite()
{
    disbleCallback();
}

bool SoftwareModuleComposite::fromJson(const JValue& json)
{
    ISerializable::fromJson(json);

    string part;
    JValueUtil::getValue(json, "part", part);
    if (!part.empty()) {
        m_type = toEnum(part);
    }
    JValueUtil::getValue(json, "name", m_name);
    JValueUtil::getValue(json, "version", m_version);
    if (json.hasKey("metadata") && json["metadata"].isArray()) {
        m_metadata = json["metadata"].duplicate();
    }
    if (json.hasKey("artifacts") && json["artifacts"].isArray()) {
        for (JValue artifact : json["artifacts"].items()) {
            shared_ptr<ArtifactLeaf> ptr = make_shared<ArtifactLeaf>();
            ptr->fromJson(artifact);
            if (m_metadata.isValid())
                ptr->setMetadata(m_metadata);
            m_children.push_back(ptr);
        }
        enableCallback();
    }
    return true;
}

bool SoftwareModuleComposite::toJson(JValue& json)
{
    Component::toJson(json);

    json.put("type", toString(m_type));
    json.put("name", m_name);
    json.put("version", m_version);

    if (m_metadata.isValid() && !m_metadata.isNull())
        json.put("metadata", m_metadata.duplicate());

    JValue artifacts = pbnjson::Array();
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        JValue artifact = pbnjson::Object();
        (*it)->toJson(artifact);
        artifacts.append(artifact);
    }
    json.put("artifacts", artifacts);
    return true;
}
