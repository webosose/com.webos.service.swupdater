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

#ifndef LS2_SETTINGSSERVICE_H_
#define LS2_SETTINGSSERVICE_H_

#include <iostream>
#include <pbnjson.hpp>
#include <luna-service2/lunaservice.hpp>

#include "interface/IInitializable.h"
#include "interface/IListener.h"
#include "interface/ISingleton.h"

using namespace std;
using namespace pbnjson;

class SettingsServiceListener {
public:
    SettingsServiceListener() {}
    virtual ~SettingsServiceListener() {}

    virtual void onGetSystemSettingsSubscription(pbnjson::JValue subscriptionPayload) = 0;
};

class SettingsService : public IInitializable,
                        public ISingleton<SettingsService> {
friend ISingleton<SettingsService>;
public:
    virtual ~SettingsService();

    // IInitializable
    virtual bool onInitialization() override;
    virtual bool onFinalization() override;

    static bool _getSystemSettings(LSHandle* sh, LSMessage* reply, void* ctx);
    bool getSystemSettings(SettingsServiceListener* listener);
    bool getSystemSettings(JValue& responsePayload);
    bool setSystemSettings(JValue& settingsObject);

private:
    SettingsService();

    static const string CATEGORY;

    LS::Call m_getSystemSettingsCall;

};

#endif /* LS2_SETTINGSSERVICE_H_ */
