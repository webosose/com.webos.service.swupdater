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

#include "ls2/ConnectionManager.h"

#include "ls2/LS2Handler.h"
#include "util/JValueUtil.h"
#include "util/Logger.h"

ConnectionManager::ConnectionManager()
{
    setClassName("ConnectionManager");
}

ConnectionManager::~ConnectionManager()
{
}

bool ConnectionManager::onInitialization()
{
    return true;
}

bool ConnectionManager::onFinalization()
{
    if (m_getStatusCall.isActive())
        m_getStatusCall.cancel();
    return true;
}

bool ConnectionManager::_getStatus(LSHandle* sh, LSMessage* reply, void* ctx)
{
    ConnectionManagerListener* listener = (ConnectionManagerListener*)ctx;
    LS::Message response(reply);
    pbnjson::JValue subscriptionPayload = JDomParser::fromString(response.getPayload());

    if (listener)
        listener->onGetStatusSubscription(subscriptionPayload);
    return true;
}

bool ConnectionManager::getStatus(ConnectionManagerListener* listener)
{
    static const string API = "luna://com.webos.service.connectionmanager/getStatus";
    pbnjson::JValue requestPayload = pbnjson::Object();
    requestPayload.put("subscribe", true);

    if (m_getStatusCall.isActive())
        m_getStatusCall.cancel();

    try {
        m_getStatusCall = LS2Handler::getInstance().callMultiReply(
            API.c_str(),
            requestPayload.stringify().c_str()
        );
        LS2Handler::writeBLog("Call", "/getStatus", requestPayload);
        m_getStatusCall.continueWith(_getStatus, listener);
    }
    catch (const LS::Error &e) {
        Logger::error(getClassName(), e.what());
    }
    return true;
}

bool ConnectionManager::getinfo(JValue& responsePayload)
{
    static const string API = "luna://com.webos.service.connectionmanager/getinfo";
    pbnjson::JValue requestPayload = pbnjson::Object();

    try {
        LS::Call call = LS2Handler::getInstance().callOneReply(
            API.c_str(),
            requestPayload.stringify().c_str()
        );
        LS2Handler::writeBLog("Call", "/getinfo", requestPayload);
        LS::Message reply = call.get(LS2Handler::LSCALL_TIMEOUT);
        if (!reply) {
            Logger::error(getClassName(), "Timeout to get MAC address");
            return false;
        }
        responsePayload = JDomParser::fromString(reply.getPayload());
        LS2Handler::writeBLog("Return", "/getinfo", responsePayload);
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
