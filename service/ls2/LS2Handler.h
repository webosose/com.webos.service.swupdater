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

#include "interface/IInitializable.h"
#include "interface/IListener.h"
#include "interface/ISingleton.h"

using namespace std;
using namespace LS;
using namespace pbnjson;

class LS2HandlerListener {
public:
    LS2HandlerListener() {};
    virtual ~LS2HandlerListener() {};

    virtual void onGetStatus(LS::Message& request, JValue& requestPayload, JValue& responsePayload) = 0;
    virtual void onInstall(LS::Message& request, JValue& requestPayload, JValue& responsePayload) = 0;
    virtual void onPause(LS::Message& request, JValue& requestPayload, JValue& responsePayload) = 0;
    virtual void onResume(LS::Message& request, JValue& requestPayload, JValue& responsePayload) = 0;
    virtual void onCancel(LS::Message& request, JValue& requestPayload, JValue& responsePayload) = 0;

};

class LS2Handler : public Handle,
                   public IInitializable,
                   public IListener<LS2HandlerListener>,
                   public ISingleton<LS2Handler> {
friend ISingleton<LS2Handler>;
public:
    static void writeALog(const string& type, LS::Message& request, JValue& payload);
    static void writeBLog(const string& type, const string& kind, JValue& payload);

    virtual ~LS2Handler();

    // IInitializable
    virtual bool onInitialization() override;
    virtual bool onFinalization() override;

private:
    static bool onRequest(LSHandle* sh, LSMessage* msg, void* category_context);

    static void before(LS::Message& request, JValue& requestPayload, JValue& responsePayload);
    static void after(LS::Message& request, JValue& requestPayload, JValue& responsePayload);

    LS2Handler();

    static const string NAME;
    static const LSMethod ROOT_METHODS[];
    static const LSMethod INSTALL_METHODS[];
    static const LSMethod DOWNLOAD_METHODS[];

    queue<LS::Message> m_requests;

};

#endif /* _MANAGER_UPDATEMANAGER_H_ */
