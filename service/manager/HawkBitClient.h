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

#include "core/HttpCall.h"
#include "core/action/CancelAction.h"
#include "core/action/InstallAction.h"
#include "interface/IInitializable.h"
#include "interface/IListener.h"
#include "interface/ISingleton.h"

using namespace pbnjson;

class HawkBitClientListener {
public:
    HawkBitClientListener() {};
    virtual ~HawkBitClientListener() {};

    virtual void onCancelUpdate(shared_ptr<CancelAction> action) = 0;
    virtual void onInstallUpdate(shared_ptr<InstallAction> action) = 0;
};

class HawkBitClient : public IInitializable,
                      public IListener<HawkBitClientListener>,
                      public ISingleton<HawkBitClient> {
friend ISingleton<HawkBitClient>;
public:
    static guint poll(gpointer data);

    virtual ~HawkBitClient();

    virtual bool onInitialization() override;
    virtual bool onFinalization() override;

    bool postComplete(shared_ptr<AbsAction> action);
    bool postProgress(shared_ptr<InstallAction> action, int of, int cnt);

    void downloadCallback(HttpCall* httpCall);
    bool downloadApplication(SoftwareModule& module);
    bool downloadOS(SoftwareModule& module);

private:
    HawkBitClient();

    void start(int seconds = 0);
    bool isStarted();
    void stop();

    void checkPollingInterval(const pbnjson::JValue& responsePayload);
    enum ActionType checkLink(const JValue& responsePayload, string& link);

    bool getRequest(JValue& responsePayload);
    bool getAction(const string& link, JValue& responsePayload);

    static const string HAWKBIT_TENANT;
    static const string HAWKBIT_URL;
    static const string HAWKBIT_ID;
    static const string HAWKBIT_TOKEN;

    static const int POLLING_INTERVAL_DEFAULT;

    string m_hawkBitUrl;
    string m_hawkBitToken;

    GSource* m_pollingSrc;
    int m_pollingInterval;

};

#endif /* MANAGER_HAWKBITCLIENT_H_ */
