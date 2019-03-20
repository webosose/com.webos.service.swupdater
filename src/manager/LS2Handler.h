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

#ifndef _MANAGER_UPDATEMANAGER_H_
#define _MANAGER_UPDATEMANAGER_H_

#include <iostream>
#include <queue>

#include <luna-service2/lunaservice.hpp>
#include <pbnjson.hpp>

#include "interface/IManageable.h"

using namespace std;
using namespace LS;
using namespace pbnjson;

class LS2Handler : public Handle, public IManageable<LS2Handler> {
friend class IManageable<LS2Handler>;
public:
    virtual ~LS2Handler();

    virtual bool onInitialization() override;
    virtual bool onFinalization() override;

    void check(LS::Message& request, JValue& requestPayload, JValue& responsePayload);
    void install(LS::Message& request, JValue& requestPayload, JValue& responsePayload);
    void cancel(LS::Message& request, JValue& requestPayload, JValue& responsePayload);
    void getStatus(LS::Message& request, JValue& requestPayload, JValue& responsePayload);

private:
    static const string NAME;
    static const LSMethod METHODS[];

    queue<LS::Message> m_requests;

    static bool onRequest(LSHandle* sh, LSMessage* msg, void* category_context);

    static void pre(LS::Message& request, JValue& requestPayload, JValue& responsePayload);
    static void post(LS::Message& request, JValue& requestPayload, JValue& responsePayload);

    LS2Handler();
};

#endif /* _MANAGER_UPDATEMANAGER_H_ */
