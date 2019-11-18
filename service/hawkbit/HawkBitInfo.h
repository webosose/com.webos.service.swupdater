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

#ifndef HAWKBIT_HAWKBITINFO_H_
#define HAWKBIT_HAWKBITINFO_H_

#include <iostream>
#include <pbnjson.hpp>

#include "interface/IInitializable.h"
#include "interface/ISingleton.h"

using namespace std;
using namespace pbnjson;

class HawkBitInfo : public IInitializable,
                    public ISingleton<HawkBitInfo> {
friend ISingleton<HawkBitInfo>;
public:
    virtual ~HawkBitInfo();

    // IInitializable
    virtual bool onInitialization() override;
    virtual bool onFinalization() override;

    bool isHawkBitInfoSet()
    {
        return m_isHawkBitInfoSet;
    }
    string getDeviceId()
    {
        return m_deviceId;
    }
    string getAddress()
    {
        return m_address;
    }
    string getToken()
    {
        return m_token;
    }
    string getTenant()
    {
        return m_tenant;;
    }
    string getBaseUrl()
    {
        return getAddress() + "/" + getTenant() + "/controller/v1/" + getDeviceId();
    }

private:
    HawkBitInfo();

    bool m_isHawkBitInfoSet;

    string m_deviceId;
    string m_address;
    string m_tenant;
    string m_token;
};

#endif /* HAWKBIT_HAWKBITINFO_H_ */
