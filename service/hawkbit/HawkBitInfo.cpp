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

#include "hawkbit/HawkBitInfo.h"

#include "Setting.h"
#include "ls2/ConnectionManager.h"
#include "util/JValueUtil.h"
#include "util/Logger.h"
#include "util/Util.h"

HawkBitInfo::HawkBitInfo()
    : m_isHawkBitInfoSet(false)
{
    setClassName("HawkBitInfo");
}

HawkBitInfo::~HawkBitInfo()
{

}

bool HawkBitInfo::onInitialization()
{
    JValue json = JDomParser::fromFile(PATH_PREFERENCE "/" FILE_HAWKBIT_INFO);
    if (json.isObject()) {
        Logger::info(getClassName(), "Load " PATH_PREFERENCE "/" FILE_HAWKBIT_INFO);
        json["deviceId"].asString(m_deviceId);
        json["address"].asString(m_address);
        json["token"].asString(m_token);
        json["tenant"].asString(m_tenant);
    }

    if (m_deviceId.empty()) {
        JValue getInfoPayload = Object();
        if (!ConnectionManager::getInstance().getinfo(getInfoPayload)) {
            Logger::info(getClassName(), "Fail to get MAC address");
            return false;
        }
        string tmp;
        if (JValueUtil::getValue(getInfoPayload, "wiredInfo", "macAddress", tmp) && !tmp.empty()) {
            m_deviceId = "webOS_" + tmp;
        } else if (JValueUtil::getValue(getInfoPayload, "wifiInfo", "macAddress", tmp) && !tmp.empty()) {
            m_deviceId = "webOS_" + tmp;
        } else {
            m_deviceId = Util::generateUuid();
        }
    }
    Logger::info(getClassName(), "deviceId: " + m_deviceId);

    // Save the generated ID to prevent it from changing every time.
    if (!json.isObject())
        json = pbnjson::Object();
    json.put("deviceId", m_deviceId);
    if (!Util::makeDir(PATH_PREFERENCE)) {
        Logger::error(getClassName(), "mkdir error: " PATH_PREFERENCE);
        return false;
    } else if (!Util::writeFile(PATH_PREFERENCE "/" FILE_HAWKBIT_INFO, json.stringify("    "))) {
        Logger::error(getClassName(), "file write error: " PATH_PREFERENCE "/" FILE_HAWKBIT_INFO);
        return false;
    }

    if (m_address.empty())
        m_address = HAWKBIT_ADDRESS;
    if (m_token.empty())
        m_token = HAWKBIT_TOKEN;
    if (m_tenant.empty())
        m_tenant = HAWKBIT_TENANT;
    Logger::info(getClassName(), "address: " + m_address);
    Logger::info(getClassName(), "tenant: " + m_tenant);

    m_isHawkBitInfoSet = !m_address.empty() && !m_token.empty() && !m_tenant.empty();
    return true;
}

bool HawkBitInfo::onFinalization()
{
    return true;
}
