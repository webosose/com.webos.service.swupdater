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

#include "interface/IManageable.h"
#include "util/HttpCall.h"

using namespace pbnjson;

class HawkBitClient : public IManageable<HawkBitClient> {
friend IManageable<HawkBitClient>;
public:
    virtual ~HawkBitClient();

    virtual bool onInitialization() override;
    virtual bool onFinalization() override;

    static guint poll(gpointer data);
    void handlePollResponse(const JValue& body);

private:
    static const string BLKEY_HAWKBIT_TENENT;
    static const string BLKEY_HAWKBIT_URL;
    static const string BLKEY_HAWKBIT_ID;
    static const string BLKEY_HAWKBIT_TOKEN;

    static const int POLLING_INTERVAL_DEFAULT;

    HawkBitClient();

    void registerPollingInterval(int seconds);
    void unregisterPollingInterval();

    GSource* m_pollingSrc;
    string m_url;
    string m_tenent;
    string m_controllerId;
    string m_token;
    int m_pollingIntervalSec;
};

#endif /* MANAGER_HAWKBITCLIENT_H_ */
