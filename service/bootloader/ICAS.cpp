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

#include "bootloader/ICAS.h"

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
        return "http://52.231.103.225";
    } else if (key == "hawkbit_token") {
        return "421fc0de2f60a06fb34e83409c806b67";
    }
    return "";
}


void ICAS::notifyUpdate()
{
}

void ICAS::setRebootOK()
{
}
