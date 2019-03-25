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

#ifndef MANAGER_HAWKBITCLIENT_H_
#define MANAGER_HAWKBITCLIENT_H_

#include <pbnjson.hpp>

#include "core/Action.h"
#include "core/Feedback.h"
#include "core/HttpCall.h"
#include "interface/IManageable.h"

using namespace pbnjson;

class HawkBitClientListener {
public:
    HawkBitClientListener() {};
    virtual ~HawkBitClientListener() {};

    virtual void onCancelUpdate(shared_ptr<Action> action) = 0;
    virtual void onInstallUpdate(shared_ptr<Action> action) = 0;
};

class HawkBitClient : public IManageable<HawkBitClient> {
friend IManageable<HawkBitClient>;
public:
    virtual ~HawkBitClient();

    virtual bool onInitialization() override;
    virtual bool onFinalization() override;

    virtual void setListener(HawkBitClientListener *listener)
    {
        m_listener = listener;
    }

    void pollOnce();
    bool feedback(Action& action, Feedback& feedback);

private:
    static guint poll(gpointer data);

    HawkBitClient();

    void start(int seconds = 0);
    bool isStarted();
    void stop();

    bool sendToServer(const string& url, HttpCall::MethodType method, const JValue& request, long& responseCode, JValue& response);

    static const string HAWKBIT_TENANT;
    static const string HAWKBIT_URL;
    static const string HAWKBIT_ID;
    static const string HAWKBIT_TOKEN;

    static const int POLLING_INTERVAL_DEFAULT;

    string m_hawkBitUrl;
    string m_hawkBitToken;

    GSource* m_pollingSrc;
    int m_pollingInterval;

    HawkBitClientListener* m_listener;
};

#endif /* MANAGER_HAWKBITCLIENT_H_ */
