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

#ifndef POLICYMANAGER_H_
#define POLICYMANAGER_H_

#include "core/AbsAction.h"
#include "hawkbit/HawkBitClient.h"
#include "hardware/AbsHardware.h"
#include "interface/IInitializable.h"
#include "interface/ISingleton.h"
#include "ls2/LS2Handler.h"

class PolicyManager : public ISingleton<PolicyManager>,
                      public IInitializable,
                      public LS2HandlerListener,
                      public HawkBitClientListener {
friend ISingleton<PolicyManager>;
public:
    virtual ~PolicyManager();

    // IInitializable
    virtual bool onInitialization() override;
    virtual bool onFinalization() override;

    // IInstallable
    virtual void onChangeStatus();

    // LS2HandlerListener
    virtual void onGetStatus(LS::Message& request, JValue& requestPayload, JValue& responsePayload) override;
    virtual void onStartDownload(LS::Message& request, JValue& requestPayload, JValue& responsePayload) override;
    virtual void onPauseDownload(LS::Message& request, JValue& requestPayload, JValue& responsePayload) override;
    virtual void onResumeDownload(LS::Message& request, JValue& requestPayload, JValue& responsePayload) override;
    virtual void onCancelDownload(LS::Message& request, JValue& requestPayload, JValue& responsePayload) override;
    virtual void onStartInstall(LS::Message& request, JValue& requestPayload, JValue& responsePayload) override;
    virtual void onPauseInstall(LS::Message& request, JValue& requestPayload, JValue& responsePayload) override;
    virtual void onResumeInstall(LS::Message& request, JValue& requestPayload, JValue& responsePayload) override;
    virtual void onCancelInstall(LS::Message& request, JValue& requestPayload, JValue& responsePayload) override;

    // HawkBitClientListener
    virtual void onCancellationAction(JValue& responsePayload) override;
    virtual void onInstallationAction(JValue& responsePayload) override;
    virtual void onConfigDataAction(JValue& responsePayload) override;

private:
    PolicyManager();

    shared_ptr<DeploymentAction> m_currentAction;
    LS::SubscriptionPoint *m_statusPoint;

};

#endif /* POLICYMANAGER_H_ */
