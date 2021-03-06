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

#ifndef LS2_APPINSTALLER_H_
#define LS2_APPINSTALLER_H_

#include <iostream>
#include <pbnjson.hpp>
#include <luna-service2/lunaservice.hpp>

#include "interface/IInitializable.h"
#include "interface/IListener.h"
#include "interface/ISingleton.h"

using namespace std;
using namespace pbnjson;

class AppInstallerListener {
public:
    AppInstallerListener() {}
    virtual ~AppInstallerListener()
    {
        if (m_call.isActive())
            m_call.cancel();
    }

    virtual void onInstallSubscription(pbnjson::JValue subscriptionPayload) = 0;

    LS::Call& getCall()
    {
        return m_call;
    }

private:
    LS::Call m_call;

};

class AppInstaller : public IInitializable,
                     public ISingleton<AppInstaller> {
friend ISingleton<AppInstaller>;
public:
    virtual ~AppInstaller();

    virtual bool onInitialization() override;
    virtual bool onFinalization() override;

    static bool _install(LSHandle* sh, LSMessage* reply, void* ctx);
    bool install(const string& id, const string& ipkUrl, AppInstallerListener* listener);

private:
    AppInstaller();

};

#endif /* LS2_APPINSTALLER_H_ */
