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

#ifndef CORE_INSTALL_SOFTWAREMODULE_H_
#define CORE_INSTALL_SOFTWAREMODULE_H_

#include <core/install/Artifact.h>
#include <iostream>
#include <list>
#include <map>
#include <pbnjson.hpp>

#include "interface/ISerializable.h"

using namespace std;
using namespace pbnjson;

enum ChunkType {
    ChunkType_Unknown,
    ChunkType_Application,
    ChunkType_OS,
    ChunkType_Mixed
};

class SoftwareModule : public ISerializable {
public:
    static string toString(enum ChunkType& type);
    static ChunkType toEnum(const string& type);

    SoftwareModule();
    virtual ~SoftwareModule();

    const enum ChunkType getType()
    {
        return m_type;
    }

    const string& getName()
    {
        return m_name;
    }

    const string& getVersion()
    {
        return m_version;
    }

    const list<Artifact>& getArtifacts()
    {
        return m_artifacts;
    }

    virtual bool fromJson(const JValue& json) override;

private:
    enum ChunkType m_type;
    string m_name;
    string m_version;
    list<Artifact> m_artifacts;
    map<string, string> m_metadata;
};


#endif /* CORE_INSTALL_SOFTWAREMODULE_H_ */
