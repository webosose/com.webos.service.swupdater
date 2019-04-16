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

#ifndef HAWKBIT_HAWKBITCLIENT_H_
#define HAWKBIT_HAWKBITCLIENT_H_

#include <pbnjson.hpp>

#include "core/install/impl/DeploymentActionComposite.h"
#include "core/HttpRequest.h"
#include "interface/IInitializable.h"
#include "interface/IListener.h"
#include "interface/ISingleton.h"

using namespace pbnjson;

class HawkBitClientListener {
public:
    HawkBitClientListener() {};
    virtual ~HawkBitClientListener() {};

    virtual void onCancellationAction(JValue& responsePayload) = 0;
    virtual void onInstallationAction(JValue& responsePayload) = 0;
    virtual void onConfigDataAction(JValue& responsePayload) = 0;
};

class HawkBitClient : public IInitializable,
                      public IListener<HawkBitClientListener>,
                      public ISingleton<HawkBitClient> {
friend ISingleton<HawkBitClient>;
public:
    static guint poll(gpointer data);

    virtual ~HawkBitClient();

    // IInitializable
    virtual bool onInitialization() override;
    virtual bool onFinalization() override;

    // See : https://www.eclipse.org/hawkbit/apis/ddi_api/
    bool canceled();
    bool rejected();
    bool closed();
    bool proceeding();
    bool scheduled();
    bool resumed();

    bool postComplete(shared_ptr<AbsAction> action);
    bool postProgress(shared_ptr<DeploymentActionComposite> action, int of, int cnt);

    // HackBit communication APIs
    bool getBase(JValue& responsePayload, const string& url);
    bool getCancellationAction(JValue& requestPayload, JValue& responsePayload, string& id);
    bool postCancellationAction(JValue& requestPayload, JValue& responsePayload, string& id);
    bool putConfigData(JValue& requestPayload, JValue& responsePayload);
    bool getDeploymentAction(JValue& requestPayload, JValue& responsePayload, string& id);
    bool postDeploymentActionSuccess(JValue& responsePayload, const string& id);
    bool postDeploymentActionFailed(JValue& responsePayload, const string& id);
    bool getSoftwaremodules(JValue& requestPayload, JValue& responsePayload, string& id);

private:
    HawkBitClient();

    void start(int seconds = 0);
    bool isStarted();
    void stop();

    static const string HAWKBIT_TENANT;
    static const string HAWKBIT_URL;
    static const string HAWKBIT_ID;

    static const int SLEEP_DEFAULT;

    string m_hawkBitUrl;

    GSource* m_pollingSrc;
    int m_sleep;

};

#endif /* HAWKBIT_HAWKBITCLIENT_H_ */
