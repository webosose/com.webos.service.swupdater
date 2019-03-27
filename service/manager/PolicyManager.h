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

#ifndef MANAGER_POLICYMANAGER_H_
#define MANAGER_POLICYMANAGER_H_

#include <hardware/AbsHardware.h>
#include "core/Action.h"
#include "interface/IManageable.h"
#include "manager/HawkBitClient.h"
#include "manager/LS2Handler.h"

class PolicyManager : public IManageable<PolicyManager>, public LS2HandlerListener, public HawkBitClientListener {
friend IManageable<PolicyManager>;
public:
    virtual ~PolicyManager();

    // IManageable
    virtual bool onInitialization() override;
    virtual bool onFinalization() override;

    // LS2HandlerListener
    virtual bool onCheck(JValue& responsePayload/**/) override;
    virtual bool onInstall(JValue& responsePayload/**/) override;
    virtual bool onCancel(JValue& responsePayload/**/) override;
    virtual bool onGetStatus(JValue& responsePayload/**/) override;

    // HawkBitClientListener
    virtual void onCancelUpdate(Action& action) override;
    virtual void onInstallUpdate(ActionInstall& action) override;

private:
    PolicyManager();

};

#endif /* MANAGER_POLICYMANAGER_H_ */
