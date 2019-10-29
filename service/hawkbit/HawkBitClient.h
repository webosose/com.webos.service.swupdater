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
    virtual void onPollingSleepAction(int seconds) = 0;
    virtual void onSettingConfigData() = 0;
};

class HawkBitClient : public IInitializable,
                      public IListener<HawkBitClientListener>,
                      public ISingleton<HawkBitClient> {
friend ISingleton<HawkBitClient>;
public:
    virtual ~HawkBitClient();

    // IInitializable
    virtual bool onInitialization() override;
    virtual bool onFinalization() override;

    void poll();

    // See : https://www.eclipse.org/hawkbit/apis/ddi_api/
    bool canceled(const string& id);
    bool rejected(const string& id);
    bool closed(const string& id);
    bool proceeding(const string& id, const string& detail);
    bool scheduled(const string& id);
    bool resumed(const string& id);

    // HackBit communication APIs
    bool postCancellationAction(const string& id, bool success);
    bool postDeploymentAction(const string& id, bool success);
    bool putConfigData(JValue& data);

private:
    HawkBitClient();

    bool getBase(JValue& responsePayload, const string& url);
    void getStatus(JValue& json, const string& execution, const string& finished, string detail = "");

};

#endif /* HAWKBIT_HAWKBITCLIENT_H_ */
