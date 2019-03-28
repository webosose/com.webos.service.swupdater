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

#include <core/AbsAction.h>
#include <hawkbit/HawkBitClient.h>
#include "hardware/AbsHardware.h"
#include "interface/IInitializable.h"
#include "interface/ISingleton.h"
#include "ls2/AppInstaller.h"
#include "ls2/LS2Handler.h"

class PolicyManager : public ISingleton<PolicyManager>,
                      public IInitializable,
                      public AppInstallerListener,
                      public LS2HandlerListener,
                      public HawkBitClientListener {
friend ISingleton<PolicyManager>;
public:
    virtual ~PolicyManager();

    // IInitializable
    virtual bool onInitialization() override;
    virtual bool onFinalization() override;

    // AppInstallerListener
    virtual void onInstallSubscription(const string& id, const string& status) override;

    // LS2HandlerListener
    virtual bool onCheck(JValue& responsePayload/**/) override;
    virtual bool onInstall(JValue& responsePayload/**/) override;
    virtual bool onCancel(JValue& responsePayload/**/) override;
    virtual bool onGetStatus(JValue& responsePayload/**/) override;

    // HawkBitClientListener
    virtual void onCancelUpdate(shared_ptr<CancelAction> action) override;
    virtual void onInstallUpdate(shared_ptr<InstallAction> action) override;

private:
    PolicyManager();

    shared_ptr<AbsAction> m_currentAction;

};

#endif /* POLICYMANAGER_H_ */
