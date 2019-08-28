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

#include <curl/curl.h>
#include <glib.h>
#include "PolicyManager.h"
#include "Setting.h"
#include "bootloader/AbsBootloader.h"
#include "core/HttpRequest.h"
#include "external/glibcurl.h"
#include "hawkbit/HawkBitClient.h"
#include "util/JValueUtil.h"
#include "util/Logger.h"
#include "util/Socket.h"
#include "util/Time.h"

const string HawkBitClient::HAWKBIT_TENANT = "hawkbit_tenant";
const string HawkBitClient::HAWKBIT_URL = "hawkbit_url";
const string HawkBitClient::HAWKBIT_ID = "hawkbit_id";

HawkBitClient::HawkBitClient()
{
    setClassName("HawkBitClient");
}

HawkBitClient::~HawkBitClient()
{
}

bool HawkBitClient::onInitialization()
{
    glibcurl_init();

    // hawkBit configuration
    string hawkBitUrl = AbsBootloader::getBootloader().getEnv(HAWKBIT_URL);
    string hawkBitTenant = AbsBootloader::getBootloader().getEnv(HAWKBIT_TENANT);
    string hawkBitId = AbsBootloader::getBootloader().getHawkBitId();

    Logger::info(getClassName(), "HawkBitInfo", hawkBitUrl);
    Logger::info(getClassName(), "HawkBitInfo", hawkBitTenant);
    Logger::info(getClassName(), "HawkBitInfo", hawkBitId);

    if (hawkBitId.empty()) {
        hawkBitId = Socket::getMacAddress("eth0");
        AbsBootloader::getBootloader().setEnv(HAWKBIT_ID, hawkBitId);
    }

    m_hawkBitUrl = hawkBitUrl + "/" + hawkBitTenant + "/controller/v1/" + hawkBitId;

    if (hawkBitUrl.empty() || hawkBitTenant.empty()) {
        Logger::error(getClassName(), "HawkBit connection info could not be found");
        return false;
    }

    return true;
}

bool HawkBitClient::onFinalization()
{
    glibcurl_cleanup();
    return true;
}

void HawkBitClient::poll()
{
    JValue responsePayload;
    string sleep = "";
    string href = "";

    Logger::info(getClassName(), "== POLLING START ==");
    if (!getBase(responsePayload, m_hawkBitUrl)) {
        goto Done;
    }

    if (JValueUtil::getValue(responsePayload, "config", "polling", "sleep", sleep)) {
        if (m_listener) m_listener->onPollingSleepAction(15); // TODO Time::toSeconds(sleep));
    }

    if (JValueUtil::getValue(responsePayload, "_links", "configData", "href", href)) {
        if (m_listener) m_listener->onSettingConfigData();
    }

    if (JValueUtil::getValue(responsePayload, "_links", "deploymentBase", "href", href)) {
        if (!getBase(responsePayload, href + "&actionHistory=10")) {
            goto Done;
        }
        if (m_listener) m_listener->onInstallationAction(responsePayload);
    }
    if (JValueUtil::getValue(responsePayload, "_links", "cancelAction", "href", href)) {
        if (!getBase(responsePayload, href)) {
            goto Done;
        }
        if (m_listener) m_listener->onCancellationAction(responsePayload);
    }

Done:
    Logger::info(getClassName(), "== POLLING END ==");
}

bool HawkBitClient::canceled(const string& id)
{
    /*
     * This is send by the target as confirmation of a cancellation request by the update server.
     */
    const string url = m_hawkBitUrl + "/cancelAction/" + id + "/feedback";
    HttpRequest httpCall;
    httpCall.open(MethodType_POST, url);

    Logger::verbose(getClassName(), "RestAPI", "POST Cancellation Action");
    return true;
}

bool HawkBitClient::rejected(const string& id)
{
    /*
     * This is send by the target in case an update of a cancellation is rejected,
     * i.e. cannot be fulfilled at this point in time.
     * Note: the target should send a CLOSED->ERROR if it believes it will not be able to proceed the action at all.
     */
    const string url = m_hawkBitUrl + "/cancelAction/" + id + "/feedback";
    HttpRequest httpCall;
    httpCall.open(MethodType_POST, url);

    Logger::verbose(getClassName(), "RestAPI", "POST Cancellation Action");
    return true;
}

bool HawkBitClient::closed(const string& id)
{
    /*
     * Target completes the action either with status.result.finished SUCCESS or FAILURE as result.
     * Note: DDI defines also a status NONE which will not be interpreted by the update server and handled like SUCCESS.
     */
    const string url = m_hawkBitUrl + "/deploymentBase/" + id + "/feedback";
    Logger::verbose(getClassName(), "RestAPI", "POST Deployment Action");

//    JValue requestPayload = pbnjson::Object();
//    requestPayload.put("id", id);
//    requestPayload.put("time", Time::getUtcTime());
//
//    if (success)
//        getStatus(requestPayload, "closed", "success");
//    else
//        getStatus(requestPayload, "closed", "failure");
//
//    HttpRequest httpCall;
//    if (!httpCall.open(MethodType_POST, url) || !httpCall.send(requestPayload)) {
//        Logger::error(getClassName(), "Failed to post feedback");
//        return false;
//    }
    return true;
}

bool HawkBitClient::proceeding(const string& id, const string& detail)
{
    /*
     * This can be used by the target to inform that it is working on the action.
     */
    const string url = m_hawkBitUrl + "/deploymentBase/" + id + "/feedback";
    Logger::verbose(getClassName(), "RestAPI", "POST Deployment Action");
    Logger::info(getClassName(), __FUNCTION__, detail);

    JValue requestPayload = pbnjson::Object();
    requestPayload.put("id", id);
    requestPayload.put("time", Time::getUtcTime());

    getStatus(requestPayload, "proceeding", "none", detail);

    HttpRequest httpCall;
    if (!httpCall.open(MethodType_POST, url) || !httpCall.send(requestPayload)) {
        Logger::error(getClassName(), "Failed to post feedback");
        return false;
    }
    return true;
}

bool HawkBitClient::scheduled(const string& id)
{
    /*
     *  This can be used by the target to inform that it scheduled on the action.
     */
    const string url = m_hawkBitUrl + "/deploymentBase/" + id + "/feedback";
    Logger::verbose(getClassName(), "RestAPI", "POST Deployment Action");

    JValue requestPayload = pbnjson::Object();
    requestPayload.put("id", id);
    requestPayload.put("time", Time::getUtcTime());

//    if (success)
//        getStatus(requestPayload, "closed", "success");
//    else
//        getStatus(requestPayload, "closed", "failure");
//
//    HttpRequest httpCall;
//    if (!httpCall.open(MethodType_POST, url) || !httpCall.send(requestPayload)) {
//        Logger::error(getClassName(), "Failed to post feedback");
//        return false;
//    }
    return true;
}

bool HawkBitClient::resumed(const string& id)
{
    /*
     * This can be used by the target to inform that it continued to work on the action.
     */
    const string url = m_hawkBitUrl + "/deploymentBase/" + id + "/feedback";
    Logger::verbose(getClassName(), "RestAPI", "POST Deployment Action");

    JValue requestPayload = pbnjson::Object();
    requestPayload.put("id", id);
    requestPayload.put("time", Time::getUtcTime());

//    if (success)
//        getStatus(requestPayload, "closed", "success");
//    else
//        getStatus(requestPayload, "closed", "failure");

    HttpRequest httpCall;
    if (!httpCall.open(MethodType_POST, url) || !httpCall.send(requestPayload)) {
        Logger::error(getClassName(), "Failed to post feedback");
        return false;
    }
    return true;
}


bool HawkBitClient::postDeploymentAction(const string& id, bool success)
{
    const string url = m_hawkBitUrl + "/deploymentBase/" + id + "/feedback";
    Logger::verbose(getClassName(), "RestAPI", "POST Deployment Action");

    JValue requestPayload = pbnjson::Object();
    requestPayload.put("id", id);
    requestPayload.put("time", Time::getUtcTime());

    if (success)
        getStatus(requestPayload, "closed", "success");
    else
        getStatus(requestPayload, "closed", "failure");

    HttpRequest httpCall;
    if (!httpCall.open(MethodType_POST, url) || !httpCall.send(requestPayload)) {
        Logger::error(getClassName(), "Failed to post feedback");
        return false;
    }
    return true;
}

bool HawkBitClient::putConfigData(JValue& data)
{
    const string url = m_hawkBitUrl + "/configData";

    JValue requestPayload = pbnjson::Object();
    requestPayload.put("time", Time::getUtcTime());
    requestPayload.put("data", data);

    getStatus(requestPayload, "closed", "success");

    HttpRequest httpCall;
    if (!httpCall.open(MethodType_PUT, url) || !httpCall.send(requestPayload)) {
        Logger::error(getClassName(), "Failed to put config data");
        return false;
    }
    return true;
}

bool HawkBitClient::getBase(JValue& responsePayload, const string& url)
{
    HttpRequest httpCall;

    Logger::verbose(getClassName(), "RestAPI", "GET " + url);
    if (!httpCall.open(MethodType_GET, url) || !httpCall.send()) {
        Logger::error(getClassName(), "Failed to perform HttpCall");
        return false;
    }

    long responseCode = httpCall.getStatus();
    if (responseCode != 200L) {
        Logger::error(getClassName(), HttpRequest::toString(responseCode));
        return false;
    }

    responsePayload = JDomParser::fromString(httpCall.getResponseText());
    Logger::verbose(getClassName(), "Response : \n" + responsePayload.stringify("    "));
    return true;
}

void HawkBitClient::getStatus(JValue& json, const string& execution, const string& finished, string detail)
{
    JValue status = pbnjson::Object();
    status.put("execution", execution);
    status.put("result", pbnjson::Object());
    status["result"].put("finished", finished);

    // TODO details is array.
    if (!detail.empty()) {
        status.put("details", pbnjson::Array());
        status["details"].append(detail);
    }

    json.put("status", status);
}
