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

#include "ls2/NotificationManager.h"

#include "ls2/LS2Handler.h"
#include "util/JValueUtil.h"
#include "util/Logger.h"

const unsigned long NotificationManager::LSCALL_TIMEOUT = 5000;

NotificationManager::NotificationManager()
{
    setClassName("NotificationManager");
}

NotificationManager::~NotificationManager()
{
}

bool NotificationManager::onInitialization()
{
    return true;
}

bool NotificationManager::onFinalization()
{
    return true;
}

bool NotificationManager::createAlert(const string& title, const string& message, JValue buttonArray)
{
    static const string API = "luna://com.webos.notification/createAlert";
    pbnjson::JValue requestPayload = pbnjson::Object();
    pbnjson::JValue responsePayload = pbnjson::Object();
    requestPayload.put("sourceId", "com.webos.service.swupdater");
    requestPayload.put("title", title);
    requestPayload.put("message", message);
    requestPayload.put("buttons", buttonArray.duplicate());

    try {
        LS::Call call = LS2Handler::getInstance().callOneReply(
            API.c_str(),
            requestPayload.stringify().c_str()
        );
        LS2Handler::writeBLog("Call", "/createAlert", requestPayload);
        LS::Message reply = call.get(LSCALL_TIMEOUT);
        if (!reply) {
            Logger::error(getClassName(), "Timeout createToast");
            return false;
        }
        responsePayload = JDomParser::fromString(reply.getPayload());
        LS2Handler::writeBLog("Return", "/createAlert", responsePayload);
        if (!responsePayload["returnValue"].asBool()) {
            Logger::error(getClassName(), responsePayload["errorText"].asString());
            return false;
        }
    }
    catch (const LS::Error &e) {
        Logger::error(getClassName(), e.what());
        return false;
    }
    return true;
}

bool NotificationManager::createToast(const string& message)
{
    static const string API = "luna://com.webos.notification/createToast";
    pbnjson::JValue requestPayload = pbnjson::Object();
    pbnjson::JValue responsePayload = pbnjson::Object();
    requestPayload.put("sourceId", "com.webos.service.swupdater");
    requestPayload.put("message", message);

    try {
        LS::Call call = LS2Handler::getInstance().callOneReply(
            API.c_str(),
            requestPayload.stringify().c_str()
        );
        LS2Handler::writeBLog("Call", "/createToast", requestPayload);
        LS::Message reply = call.get(LSCALL_TIMEOUT);
        if (!reply) {
            Logger::error(getClassName(), "Timeout createToast");
            return false;
        }
        responsePayload = JDomParser::fromString(reply.getPayload());
        LS2Handler::writeBLog("Return", "/createToast", responsePayload);
        if (!responsePayload["returnValue"].asBool()) {
            Logger::error(getClassName(), responsePayload["errorText"].asString());
            return false;
        }
    }
    catch (const LS::Error &e) {
        Logger::error(getClassName(), e.what());
        return false;
    }
    return true;
}
