// Copyright (c) 2019-2021 LG Electronics, Inc.
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

#ifndef CORE_INSTALL_IMPL_DEPLOYMENTACTIONCOMPOSITE_H_
#define CORE_INSTALL_IMPL_DEPLOYMENTACTIONCOMPOSITE_H_

#include <iostream>
#include <deque>
#include <map>

#include "core/AbsAction.h"
#include "core/install/design/Composite.h"
#include "core/install/impl/SoftwareModuleComposite.h"
#include "interface/IClassName.h"
#include "interface/IListener.h"
#include "interface/ISerializable.h"

using namespace std;

class DeploymentActionComposite : public AbsAction,
                                  public Composite,
                                  public CompositeListener,
                                  public IListener<CompositeListener> {
public:
    DeploymentActionComposite();
    virtual ~DeploymentActionComposite();

    // ISerializable
    virtual bool fromJson(const JValue& json) override;
    virtual bool toJson(JValue& json) override;

    // CompositeListener
    virtual void onChangedStatus(Composite* softwareModule) override;
    virtual void onCompletedDownload(Composite* softwareModule) override;
    virtual void onCompletedInstall(Composite* softwareModule) override;
    virtual void onFailedDownload(Composite* softwareModule) override;
    virtual void onFailedInstall(Composite* softwareModule) override;

    // Composite
    virtual bool startDownload() override;
    virtual bool pauseDownload() override;
    virtual bool resumeDownload() override;
    virtual bool cancelDownload() override;
    virtual bool startInstall() override;
    virtual bool cancelInstall() override;

    bool isForceDownload()
    {
        return m_isForceDownload;
    }

    bool isForceUpdate()
    {
        return m_isForceUpdate;
    }

    bool fromActionHistory(const JValue& json);
    bool toActionHistory(JValue& json);
    bool createRebootAlert(SoftwareModuleType type);
    void removeDownloadedFiles();

    Status& getStatus()
    {
        return m_status;
    }

    static const string FILE_NON_VOLITILE_REBOOTCHECK;
    static const string FILE_VOLITILE_REBOOTCHECK;

private:
    bool setStatus(enum StatusType status, bool doFeedback = true);

    bool m_isForceDownload;
    bool m_isForceUpdate;

    Status m_status;

};

#endif /* CORE_INSTALL_IMPL_DEPLOYMENTACTIONCOMPOSITE_H_ */
