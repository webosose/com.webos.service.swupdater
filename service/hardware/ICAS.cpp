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

#include "ICAS.h"
#include "Setting.h"

ICAS::ICAS()
{
}

ICAS::~ICAS()
{
}

void ICAS::setEnv(const string& key, const string& value)
{
    // nothing
}

string ICAS::getEnv(const string& key)
{
    if (key == "hawkbit_tenant") {
        return "DEFAULT";
    } else if (key == "hawkbit_url") {
        return "https://maas-ota-server.apps.na.vwapps.io";
    } else if (key == "hawkbit_id") {
        if (Setting::getInstance().getId().empty())
            return "webOS_Demo";
        else
            return Setting::getInstance().getId();
    } else if (key == "hawkbit_token") {
        return "007eecfb134343209cb882d45becd8d7";
    }
    return "";
}
