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

#include "bootloader/AbsBootloader.h"
#include "core/AbsAction.h"
#include "core/install/impl/DeploymentActionComposite.h"
#include "hawkbit/HawkBitClient.h"
#include "interface/IInitializable.h"
#include "interface/ISingleton.h"
#include "ls2/ConnectionManager.h"
#include "ls2/LS2Handler.h"
#include "ls2/SettingsService.h"

class PolicyManager : public ISingleton<PolicyManager>,
                      public IInitializable,
                      public ConnectionManagerListener,
                      public LS2HandlerListener,
                      public HawkBitClientListener,
                      public SettingsServiceListener,
                      public DeploymentActionCompositeListener {
friend ISingleton<PolicyManager>;
public:
    static gboolean _tick(gpointer user_data);

    virtual ~PolicyManager();

    // IInitializable
    virtual bool onInitialization() override;
    virtual bool onFinalization() override;

    // global events
    virtual void onRequestStatusChange();
    virtual void onRequestProgressUpdate();

    // ConnectionManagerListener
    virtual void onGetStatusSubscription(pbnjson::JValue subscriptionPayload) override;

    // SettingsServiceListener
    virtual void onGetSystemSettingsSubscription(pbnjson::JValue subscriptionPayload) override;

    // LS2HandlerListener
    virtual void onConnect(LS::Message& request, JValue& requestPayload, JValue& responsePayload) override;
    virtual void onGetStatus(LS::Message& request, JValue& requestPayload, JValue& responsePayload) override;
    virtual void onSetConfig(LS::Message& request, JValue& requestPayload, JValue& responsePayload) override;
    virtual void onStartDownload(LS::Message& request, JValue& requestPayload, JValue& responsePayload) override;
    virtual void onPauseDownload(LS::Message& request, JValue& requestPayload, JValue& responsePayload) override;
    virtual void onResumeDownload(LS::Message& request, JValue& requestPayload, JValue& responsePayload) override;
    virtual void onCancelDownload(LS::Message& request, JValue& requestPayload, JValue& responsePayload) override;
    virtual void onStartInstall(LS::Message& request, JValue& requestPayload, JValue& responsePayload) override;
    virtual void onCancelInstall(LS::Message& request, JValue& requestPayload, JValue& responsePayload) override;

    // HawkBitClientListener
    virtual void onCancellationAction(JValue& responsePayload) override;
    virtual void onInstallationAction(JValue& responsePayload) override;
    virtual void onPollingSleepAction(int seconds) override;
    virtual void onSettingConfigData() override;

    // DeploymentActionCompositeListener
    virtual void onChangedStatus(DeploymentActionComposite* deploymentAction) override;
    virtual void onCompletedDownload(DeploymentActionComposite* deploymentAction) override;
    virtual void onCompletedInstall(DeploymentActionComposite* deploymentAction) override;
    virtual void onFailedDownload(DeploymentActionComposite* deploymentAction) override;
    virtual void onFailedInstall(DeploymentActionComposite* deploymentAction) override;

private:
    PolicyManager();

    void postStatus();

    static const int DEFAULT_TICK_INTERVAL = 15;
    static const string FILE_NON_VOLITILE_REBOOTCHECK;
    static const string FILE_VOLITILE_REBOOTCHECK;

    shared_ptr<DeploymentActionComposite> m_currentAction;
    JValue m_proceedingJson;
    LS::SubscriptionPoint *m_statusPoint;

    int m_tickInterval;
    guint m_tickSrc;

    // TODO this is a temp solution. it should be changed *queue* before polling
    bool m_pendingClearRequest;

    bool m_isAutoUpdateOn;
};

#endif /* POLICYMANAGER_H_ */
