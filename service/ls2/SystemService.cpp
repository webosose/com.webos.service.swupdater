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

#include "SystemService.h"

#include "LS2Handler.h"
#include "util/Logger.h"

const string SystemService::SERVICE_NAME = "com.webos.service.systemservice";
const unsigned long SystemService::LSCALL_TIMEOUT = 5000;

SystemService::SystemService()
{
    setClassName("SystemService");
}

SystemService::~SystemService()
{
}

bool SystemService::queryOSInfo(JValue& responsePayload)
{
    static const string API = "luna://" + SERVICE_NAME + "/osInfo/query";
    pbnjson::JValue requestPayload = pbnjson::Object();
    pbnjson::JValue parameters = pbnjson::Array();
    parameters.append("webos_build_id");
    parameters.append("webos_release_codename");
    requestPayload.put("parameters", parameters);

    try {
        LS::Call call = LS2Handler::getInstance().callOneReply(
            API.c_str(),
            requestPayload.stringify().c_str()
        );
        LS2Handler::writeBLog("Call", "/osInfo/query", requestPayload);
        LS::Message reply = call.get(LSCALL_TIMEOUT);
        if (!reply) {
            Logger::error(getClassName(), "Timeout OS info query");
            return false;
        }
        responsePayload = JDomParser::fromString(reply.getPayload());
        LS2Handler::writeBLog("Return", "/osInfo/query", responsePayload);
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

bool SystemService::queryDeviceInfo(JValue& responsePayload)
{
    static const string API = "luna://" + SERVICE_NAME + "/deviceInfo/query";
    pbnjson::JValue requestPayload = pbnjson::Object();
    pbnjson::JValue parameters = pbnjson::Array();
    parameters.append("device_name");
    requestPayload.put("parameters", parameters);

    try {
        LS::Call call = LS2Handler::getInstance().callOneReply(
            API.c_str(),
            requestPayload.stringify().c_str()
        );
        LS2Handler::writeBLog("Call", "/deviceInfo/query", requestPayload);
        LS::Message reply = call.get(LSCALL_TIMEOUT);
        if (!reply) {
            Logger::error(getClassName(), "Timeout device info query");
            return false;
        }
        responsePayload = JDomParser::fromString(reply.getPayload());
        LS2Handler::writeBLog("Return", "/deviceInfo/query", responsePayload);
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
