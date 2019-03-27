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

#include "AppInstaller.h"

#include "LS2Handler.h"
#include "util/Logger.h"

AppInstaller::AppInstaller()
{
    setClassName("Appinstaller");
}

AppInstaller::~AppInstaller()
{
}

bool AppInstaller::_install(LSHandle* sh, LSMessage* reply, void* ctx)
{
    Message response(reply);
    pbnjson::JValue responsePayload = JDomParser::fromString(response.getPayload());

    if (AppInstaller::getInstance().getListener() == nullptr)
        return true;

    string id;
    string state;

    if (!responsePayload.hasKey("id") || responsePayload["id"].asString(id) != CONV_OK) {
        Logger::warning(AppInstaller::getInstance().getClassName(), "Failed to parse 'id' in 'install' subscription");
        return true;
    }
    if (!responsePayload.hasKey("details") ||
        !responsePayload["details"].hasKey("state") ||
        responsePayload["details"]["state"].asString(state) != CONV_OK) {
        Logger::warning(AppInstaller::getInstance().getClassName(), "Failed to parse 'state' in 'install' subscription");
        return true;
    }
    AppInstaller::getInstance().getListener()->onInstallSubscription(id, state);
    return true;
}

bool AppInstaller::onInitialization()
{
    return true;
}
bool AppInstaller::onFinalization()
{
    if (m_install.isActive())
        m_install.cancel();
    return true;
}

bool AppInstaller::install(const string& id, const string& ipkUrl)
{
    static const string API = "luna://com.webos.appInstallService/install";
    pbnjson::JValue requestPayload = pbnjson::Object();
    requestPayload.put("id", id);
    requestPayload.put("ipkUrl", ipkUrl);
    requestPayload.put("subscribe", true);

    if (m_install.isActive())
        m_install.cancel();

    try {
        m_install = LS2Handler::getInstance().callMultiReply(
            API.c_str(),
            requestPayload.stringify().c_str()
        );
        Logger::debug(getClassName(), "Call", "listApps");
        m_install.continueWith(_install, this);
    }
    catch (const LS::Error &e) {
        Logger::error(getClassName(), e.what());
    }
    return true;
}
