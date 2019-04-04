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

#include <core/State.h>
#include <iostream>
#include <deque>
#include <map>

#include "core/AbsAction.h"
#include "core/install/SoftwareModule.h"

using namespace std;

class DeploymentAction : public AbsAction {
public:
    DeploymentAction(JValue& json);
    virtual ~DeploymentAction();

    bool prepare();
    bool start();
    bool pause();
    bool resume();
    bool cancel();

    void onDownloadStateChanged(enum StateType prev, enum StateType cur, void *source);
    bool prepareDownload();
    bool startDownload();
    bool pauseDownload();
    bool resumeDownload();
    bool cancelDownload();

    void onUpdateStateChanged(enum StateType prev, enum StateType cur, void *source);
    bool prepareUpdate();
    bool startUpdate();
    bool pauseUpdate();
    bool resumeUpdate();
    bool cancelUpdate();

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

    bool isComplete()
    {
        return (m_download.getState() == StateType_COMPLETED && m_update.getState() == StateType_COMPLETED);
    }

    bool isFailed()
    {
        return (m_download.getState() == StateType_FAILED || m_update.getState() == StateType_FAILED);
    }

    State& getDownloadState()
    {
        return m_download;
    }

    State& getUpdateState()
    {
        return m_update;
    }

private:
    void addCallback();
    void removeCallback();

    bool m_isForceDownload;
    bool m_isForceUpdate;

    State m_download;
    State m_update;

    deque<SoftwareModule> m_modules;
    unsigned int m_curDownload;
    unsigned int m_curUpdate;
};

#endif /* CORE_INSTALL_DEPLOYMENTACTION_H_ */
