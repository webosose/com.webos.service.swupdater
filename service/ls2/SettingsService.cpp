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

#include "ls2/SettingsService.h"

#include "ls2/LS2Handler.h"
#include "util/JValueUtil.h"
#include "util/Logger.h"

const string SettingsService::CATEGORY = "swupdate";

SettingsService::SettingsService()
{
    setClassName("SettingsService");
}

SettingsService::~SettingsService()
{
}

bool SettingsService::onInitialization()
{
    return true;
}

bool SettingsService::onFinalization()
{
    if (m_getSystemSettingsCall.isActive())
        m_getSystemSettingsCall.cancel();
    return true;
}

bool SettingsService::_getSystemSettings(LSHandle* sh, LSMessage* reply, void* ctx)
{
    SettingsServiceListener* listener = (SettingsServiceListener*)ctx;
    LS::Message response(reply);
    pbnjson::JValue subscriptionPayload = JDomParser::fromString(response.getPayload());

    if (listener)
        listener->onGetSystemSettingsSubscription(subscriptionPayload);
    return true;
}

bool SettingsService::getSystemSettings(SettingsServiceListener* listener)
{
    static const string API = "luna://com.webos.service.settings/getSystemSettings";
    pbnjson::JValue requestPayload = pbnjson::Object();
    requestPayload.put("subscribe", true);
    requestPayload.put("category", CATEGORY);

    if (m_getSystemSettingsCall.isActive())
        m_getSystemSettingsCall.cancel();

    try {
        m_getSystemSettingsCall = LS2Handler::getInstance().callMultiReply(
            API.c_str(),
            requestPayload.stringify().c_str()
        );
        LS2Handler::writeBLog("Call", "/getSystemSettings", requestPayload);
        m_getSystemSettingsCall.continueWith(_getSystemSettings, listener);
    }
    catch (const LS::Error &e) {
        Logger::error(getClassName(), e.what());
    }
    return true;
}

bool SettingsService::getSystemSettings(JValue& responsePayload)
{
    static const string API = "luna://com.webos.service.settings/getSystemSettings";
    pbnjson::JValue requestPayload = pbnjson::Object();
    requestPayload.put("category", CATEGORY);

    try {
        LS::Call call = LS2Handler::getInstance().callOneReply(
            API.c_str(),
            requestPayload.stringify().c_str()
        );
        LS2Handler::writeBLog("Call", "/getSystemSettings", requestPayload);
        LS::Message reply = call.get(LS2Handler::LSCALL_TIMEOUT);
        if (!reply) {
            Logger::error(getClassName(), "Timeout to get configs");
            return false;
        }
        responsePayload = JDomParser::fromString(reply.getPayload());
        LS2Handler::writeBLog("Return", "/getSystemSettings", responsePayload);
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

bool SettingsService::setSystemSettings(JValue& settingsObject)
{
    static const string API = "luna://com.webos.service.settings/setSystemSettings";
    pbnjson::JValue requestPayload = pbnjson::Object();
    pbnjson::JValue responsePayload = pbnjson::Object();
    requestPayload.put("category", CATEGORY);
    requestPayload.put("settings", settingsObject.duplicate());

    try {
        LS::Call call = LS2Handler::getInstance().callOneReply(
            API.c_str(),
            requestPayload.stringify().c_str()
        );
        LS2Handler::writeBLog("Call", "/setSystemSettings", requestPayload);
        LS::Message reply = call.get(LS2Handler::LSCALL_TIMEOUT);
        if (!reply) {
            Logger::error(getClassName(), "Timeout setSystemSettings");
            return false;
        }
        responsePayload = JDomParser::fromString(reply.getPayload());
        LS2Handler::writeBLog("Return", "/setSystemSettings", responsePayload);
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
