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

#include "LS2Handler.h"

#include <lunaservice.h>

#include "util/Logger.h"

const string LS2Handler::NAME = "com.webos.service.swupdater";
const LSMethod LS2Handler::METHODS[] = {
    { "check", LS2Handler::onRequest , LUNA_METHOD_FLAGS_NONE },
    { "install", LS2Handler::onRequest , LUNA_METHOD_FLAGS_NONE },
    { "cancel", LS2Handler::onRequest , LUNA_METHOD_FLAGS_NONE },
    { "getStatus", LS2Handler::onRequest , LUNA_METHOD_FLAGS_NONE },
    { 0, 0 , LUNA_METHOD_FLAGS_NONE }
};

bool LS2Handler::onRequest(LSHandle *sh, LSMessage *msg, void *category_context)
{
    static bool pending = false;

    // All LS2 requests are handled in queue
    LS2Handler::getInstance().m_requests.emplace(msg);

    // Not allowed recursive request handling To avoid unexpected behavior
    if (pending) {
        return true;
    }

    JValue requestPayload, responsePayload;
    pending = true;
    while (LS2Handler::getInstance().m_requests.size() != 0) {
        // pop a request
        LS::Message request = LS2Handler::getInstance().m_requests.front();
        LS2Handler::getInstance().m_requests.pop();

        // pre processing before request handling
        pre(request, requestPayload, responsePayload);
        string method = request.getMethod();

        if (method == "register") {
            LS2Handler::getInstance().check(request, requestPayload, responsePayload);
        } else if (method == "unregister") {
            LS2Handler::getInstance().install(request, requestPayload, responsePayload);
        } else if (method == "resolve") {
            LS2Handler::getInstance().cancel(request, requestPayload, responsePayload);
        } else if (method == "check") {
            LS2Handler::getInstance().getStatus(request, requestPayload, responsePayload);
        } else {
            responsePayload.put("errorText", "Please extend API handlers");
        }
        post(request, requestPayload, responsePayload);
    }
    pending = false;
    return true;
}

LS2Handler::LS2Handler()
    : Handle(LS::registerService(NAME.c_str()))
{
    setName("LS2Handler");
    this->registerCategory("/", METHODS, NULL, NULL);
}

LS2Handler::~LS2Handler()
{
}

bool LS2Handler::onInitialization()
{
    attachToLoop(m_mainloop);
    return true;
}

bool LS2Handler::onFinalization()
{
    detach();
    return true;
}

void LS2Handler::pre(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    requestPayload = JDomParser::fromString(request.getPayload());
    responsePayload = pbnjson::Object();

    Logger::info(NAME, "Request", request.getMethod());
}

void LS2Handler::post(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (responsePayload.hasKey("errorText")) {
        Logger::warning(NAME, responsePayload["errorText"].asString());
        responsePayload.put("returnValue", false);
    }
    if (!responsePayload.hasKey("returnValue")) {
        responsePayload.put("returnValue", true);
    }
    request.respond(responsePayload.stringify().c_str());

    Logger::info(NAME, "Response", request.getMethod());
}

void LS2Handler::check(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
}

void LS2Handler::install(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
}

void LS2Handler::cancel(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
}

void LS2Handler::getStatus(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
}
