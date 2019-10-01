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

#ifndef LS2_CONNECTIONMANAGER_H_
#define LS2_CONNECTIONMANAGER_H_

#include <iostream>
#include <pbnjson.hpp>
#include <luna-service2/lunaservice.hpp>

#include "interface/IInitializable.h"
#include "interface/ISingleton.h"

using namespace std;
using namespace pbnjson;

class ConnectionManagerListener {
public:
    ConnectionManagerListener() {}
    virtual ~ConnectionManagerListener() {}

    virtual void onGetStatusSubscription(pbnjson::JValue subscriptionPayload) = 0;
};

class ConnectionManager : public IInitializable,
                          public ISingleton<ConnectionManager> {
friend ISingleton<ConnectionManager>;
public:
    virtual ~ConnectionManager();

    // IInitializable
    virtual bool onInitialization() override;
    virtual bool onFinalization() override;

    static bool _getStatus(LSHandle* sh, LSMessage* reply, void* ctx);
    bool getStatus(ConnectionManagerListener* listener);
    bool getinfo(JValue& responsePayload);

private:
    ConnectionManager();

    LS::Call m_getStatusCall;

};

#endif /* LS2_CONNECTIONMANAGER_H_ */
