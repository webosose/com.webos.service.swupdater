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

#ifndef CORE_INSTALL_IMPL_SOFTWAREMODULECOMPOSITE_H_
#define CORE_INSTALL_IMPL_SOFTWAREMODULECOMPOSITE_H_

#include <iostream>
#include <deque>
#include <pbnjson.hpp>

#include "core/install/design/Composite.h"
#include "core/install/impl/ArtifactLeaf.h"
#include "core/Status.h"
#include "interface/IClassName.h"
#include "interface/IListener.h"
#include "interface/ISerializable.h"

using namespace std;
using namespace pbnjson;

enum SoftwareModuleType {
    SoftwareModuleType_Unknown,
    SoftwareModuleType_Application,
    SoftwareModuleType_OS,
    SoftwareModuleType_Mixed
};

class SoftwareModuleComposite;

class SoftwareModuleCompositeListener {
public:
    SoftwareModuleCompositeListener() {}
    virtual ~SoftwareModuleCompositeListener() {}

    virtual void onChangedStatus(SoftwareModuleComposite* softwareModule) = 0;
    virtual void onCompletedDownload(SoftwareModuleComposite* softwareModule) = 0;
    virtual void onCompletedInstall(SoftwareModuleComposite* softwareModule) = 0;
    virtual void onFailedDownload(SoftwareModuleComposite* softwareModule) = 0;
    virtual void onFailedInstall(SoftwareModuleComposite* softwareModule) = 0;
};

class SoftwareModuleComposite : public IClassName,
                                public Composite,
                                public ArtifactLeafListener,
                                public IListener<SoftwareModuleCompositeListener> {
public:
    static string toString(enum SoftwareModuleType& type);
    static SoftwareModuleType toEnum(const string& type);

    SoftwareModuleComposite();
    virtual ~SoftwareModuleComposite();

    // ISerializable
    virtual bool fromJson(const JValue& json) override;
    virtual bool toJson(JValue& json) override;

    // ArtifactLeafListener
    virtual void onChangedStatus(ArtifactLeaf* artifact) override;
    virtual void onCompletedDownload(ArtifactLeaf* artifact) override;
    virtual void onCompletedInstall(ArtifactLeaf* artifact) override;
    virtual void onFailedDownload(ArtifactLeaf* artifact) override;
    virtual void onFailedInstall(ArtifactLeaf* artifact) override;

    // LS2 methods : Composite
    virtual bool startDownload() override;
    virtual bool pauseDownload() override;
    virtual bool resumeDownload() override;
    virtual bool cancelDownload() override;

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

    JValue& getMetadata()
    {
        return m_metadata;
    }

    bool restore(StatusType lastStatus);

protected:
    enum SoftwareModuleType m_type;
    string m_name;
    string m_version;

    JValue m_metadata;

};


#endif /* CORE_INSTALL_IMPL_SOFTWAREMODULECOMPOSITE_H_ */
