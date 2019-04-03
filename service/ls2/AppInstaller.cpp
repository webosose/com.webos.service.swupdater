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
    AppInstallerListener* listener = (AppInstallerListener*)ctx;
    Message response(reply);
    pbnjson::JValue subscriptionPayload = JDomParser::fromString(response.getPayload());

    if (listener)
        listener->onInstallSubscription(subscriptionPayload);
    return true;
}

bool AppInstaller::onInitialization()
{
    return true;
}
bool AppInstaller::onFinalization()
{
    return true;
}

bool AppInstaller::install(const string& id, const string& ipkUrl, AppInstallerListener* listener)
{
    static const string API = "luna://com.webos.appInstallService/install";
    pbnjson::JValue requestPayload = pbnjson::Object();
    requestPayload.put("id", id);
    requestPayload.put("ipkUrl", ipkUrl);
    requestPayload.put("subscribe", true);

    if (listener->getCall().isActive())
        listener->getCall().cancel();

    try {
        listener->getCall() = LS2Handler::getInstance().callMultiReply(
            API.c_str(),
            requestPayload.stringify().c_str()
        );
        Logger::debug(getClassName(), "Call", "install");
        listener->getCall().continueWith(_install, listener);
    }
    catch (const LS::Error &e) {
        Logger::error(getClassName(), e.what());
    }
    return true;
}
