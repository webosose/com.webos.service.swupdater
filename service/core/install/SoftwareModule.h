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

#include <core/State.h>
#include <iostream>
#include <list>
#include <map>
#include <pbnjson.hpp>

#include "core/install/Artifact.h"
#include "interface/IClassName.h"
#include "interface/ISerializable.h"
#include "interface/ISingleton.h"
#include "interface/IListener.h"

using namespace std;
using namespace pbnjson;

enum SoftwareModuleType {
    SoftwareModuleType_Unknown,
    SoftwareModuleType_Application,
    SoftwareModuleType_OS,
    SoftwareModuleType_Mixed
};

class SoftwareModule;

class SoftwareModule : public IClassName,
                       public ISerializable {
public:
    static shared_ptr<SoftwareModule> createSoftwareModule(JValue& json);

    static string toString(enum SoftwareModuleType& type);
    static SoftwareModuleType toEnum(const string& type);

    SoftwareModule();
    virtual ~SoftwareModule();

    virtual bool startUpdate() = 0;

    bool ready(bool download);
    bool start(bool download);
    bool pause(bool download);
    bool resume(bool download);
    bool cancel(bool download);

    virtual void onDownloadStateChanged(State *installer, enum StateType prev, enum StateType cur);

    // ISerializable
    virtual bool fromJson(const JValue& json) override;
    virtual bool toJson(JValue& json) override;

    const enum SoftwareModuleType getType()
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

    State& getDownloadState()
    {
        return m_downloadState;
    }

    State& getUpdateState()
    {
        return m_updateState;
    }

protected:
    enum SoftwareModuleType m_type;
    string m_name;
    string m_version;

    State m_downloadState;
    State m_updateState;

    list<Artifact> m_artifacts;

};


#endif /* CORE_INSTALL_SOFTWAREMODULE_H_ */
