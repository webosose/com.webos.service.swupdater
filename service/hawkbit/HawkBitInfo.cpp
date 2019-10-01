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

#include "util/JValueUtil.h"
#include "util/Logger.h"
#include "util/Util.h"

const string HawkBitInfo::PATH_PREFERENCE = "/var/preferences/com.webos.service.swupdater/";
const string HawkBitInfo::FILENAME_HAWKBIT_INFO = "hawkBitInfo.json";
const string HawkBitInfo::TENANT_DEFAULT = "DEFAULT";

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
    JValue json = JDomParser::fromFile((PATH_PREFERENCE + FILENAME_HAWKBIT_INFO).c_str());
    if (json.isNull()) {
        Logger::info(getClassName(), "Could not find hawkBit connection info");
        return false;
    }

    m_json = json;
    m_isHawkBitInfoSet = true;
    return true;
}

bool HawkBitInfo::onFinalization()
{
    return true;
}

bool HawkBitInfo::setJson(const JValue& json)
{
    JValue tmpJson = json;
    string tmpStr;
    if (!JValueUtil::getValue(tmpJson, "tenant", tmpStr) || tmpStr.empty()) {
        tmpJson.put("tenant", TENANT_DEFAULT);
    }

    if (!Util::makeDir(PATH_PREFERENCE)) {
        Logger::error(getClassName(), "mkdir error: " + PATH_PREFERENCE);
        return false;
    }
    if (!Util::writeFile(PATH_PREFERENCE + FILENAME_HAWKBIT_INFO, tmpJson.stringify("    "))) {
        Logger::error(getClassName(), "file write error: " + PATH_PREFERENCE + FILENAME_HAWKBIT_INFO);
        return false;
    }

    Logger::info(getClassName(), "HawkBitInfo is set");
    m_json = tmpJson.duplicate();
    m_isHawkBitInfoSet = true;
    return true;
}
