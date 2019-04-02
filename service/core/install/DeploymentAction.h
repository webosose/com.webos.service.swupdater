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

#ifndef CORE_INSTALL_DEPLOYMENTACTION_H_
#define CORE_INSTALL_DEPLOYMENTACTION_H_

#include <interface/IInstaller.h>
#include <iostream>
#include <list>
#include <map>

#include "core/AbsAction.h"
#include "core/install/SoftwareModule.h"

using namespace std;

class DeploymentAction : public AbsAction,
                         public IInstaller {
public:
    DeploymentAction(JValue& json);
    virtual ~DeploymentAction();

    // IInstaller
    virtual bool ready() override;
    virtual bool start() override;
    virtual void onStateChange(IInstaller *installer, enum InstallerState prev, enum InstallerState cur) override;

    // ISerializable
    virtual bool fromJson(const JValue& json) override;
    virtual bool toJson(JValue& json) override;

    const bool isForceDownload()
    {
        return m_isForceDownload;
    }

    const bool isForceUpdate()
    {
        return m_isForceUpdate;
    }

private:
    bool m_isForceDownload;
    bool m_isForceUpdate;

    list<shared_ptr<SoftwareModule>> m_softwareModules;
};

#endif /* CORE_INSTALL_DEPLOYMENTACTION_H_ */
