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

#include "Setting.h"
#include "core/HttpRequest.h"
#include "external/glibcurl.h"
#include "hardware/AbsHardware.h"
#include "hawkbit/HawkBitClient.h"
#include "util/JValueUtil.h"
#include "util/Logger.h"
#include "util/Socket.h"
#include "util/Time.h"

const string HawkBitClient::HAWKBIT_TENANT = "hawkbit_tenant";
const string HawkBitClient::HAWKBIT_URL = "hawkbit_url";
const string HawkBitClient::HAWKBIT_ID = "hawkbit_id";

const int HawkBitClient::SLEEP_DEFAULT = 15;

HawkBitClient::HawkBitClient()
    : m_pollingSrc(nullptr)
    , m_sleep(-1)
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
    string hawkBitUrl = AbsHardware::getHardware().getEnv(HAWKBIT_URL);
    string hawkBitTenant = AbsHardware::getHardware().getEnv(HAWKBIT_TENANT);
    string hawkBitId = AbsHardware::getHardware().getEnv(HAWKBIT_ID);

    if (hawkBitId.empty()) {
        hawkBitId = Socket::getMacAddress("eth0");
        AbsHardware::getHardware().setEnv(HAWKBIT_ID, hawkBitId);
    }

    m_hawkBitUrl = hawkBitUrl + "/" + hawkBitTenant + "/controller/v1/" + hawkBitId;

    if (hawkBitUrl.empty() || hawkBitTenant.empty()) {
        Logger::error(getClassName(), "HawkBit connection info could not be found");
        return false;
    }

    start(SLEEP_DEFAULT);
    return true;
}

bool HawkBitClient::onFinalization()
{
    stop();
    glibcurl_cleanup();
    return true;
}

bool HawkBitClient::canceled()
{
    /*
     * This is send by the target as confirmation of a cancellation request by the update server.
     */
    return true;
}

bool HawkBitClient::rejected()
{
    /*
     * This is send by the target in case an update of a cancellation is rejected,
     * i.e. cannot be fulfilled at this point in time.
     * Note: the target should send a CLOSED->ERROR if it believes it will not be able to proceed the action at all.
     */
    return true;
}

bool HawkBitClient::closed()
{
    /*
     * Target completes the action either with status.result.finished SUCCESS or FAILURE as result.
     * Note: DDI defines also a status NONE which will not be interpreted by the update server and handled like SUCCESS.
     */
    return true;
}

bool HawkBitClient::proceeding()
{
    /*
     * This can be used by the target to inform that it is working on the action.
     */
    return true;
}

bool HawkBitClient::scheduled()
{
    /*
     *  This can be used by the target to inform that it scheduled on the action.
     */
    return true;
}

bool HawkBitClient::resumed()
{
    /*
     * This can be used by the target to inform that it continued to work on the action.
     */
    return true;
}

bool HawkBitClient::postComplete(shared_ptr<AbsAction> action)
{
    JValue requestPayload = pbnjson::Object();

    requestPayload.put("id", action->getId());
    requestPayload.put("time", Time::getUtcTime());
    requestPayload.put("status", pbnjson::Object());
    requestPayload["status"].put("execution", "closed");
    requestPayload["status"].put("result", pbnjson::Object());
    requestPayload["status"]["result"].put("finished", "success");

    string url;
    if (action->getType() == ActionType_INSTALL) {
        url = m_hawkBitUrl + "/deploymentBase/" + action->getId() + "/feedback";
    } else {
        url = m_hawkBitUrl + "/cancelAction/" + action->getId() + "/feedback";
    }

    HttpRequest httpCall;
    if (!httpCall.open(MethodType_POST, url) || !httpCall.send(requestPayload)) {
        Logger::error(getClassName(), "Failed to post feedback");
        return false;
    }
    return true;
}

bool HawkBitClient::postProgress(shared_ptr<DeploymentAction> action, int of, int cnt)
{
    const string url = m_hawkBitUrl + "/deploymentBase/" + action->getId() + "/feedback";

    JValue requestPayload = pbnjson::Object();
    requestPayload.put("id", action->getId());
    requestPayload.put("time", Time::getUtcTime());
    requestPayload.put("status", pbnjson::Object());
    requestPayload["status"].put("execution", "proceeding");
    requestPayload["status"].put("result", pbnjson::Object());
    requestPayload["status"]["result"].put("finished", "none");
    requestPayload["status"]["result"].put("progress", pbnjson::Object());
    requestPayload["status"]["result"]["progress"].put("of", of);
    requestPayload["status"]["result"]["progress"].put("cnt", cnt);

    HttpRequest httpCall;
    if (httpCall.open(MethodType_POST, url) || !httpCall.send(requestPayload)) {
        Logger::error(getClassName(), "Failed to post feedback");
        return false;
    }
    return true;
}

void HawkBitClient::start(int sleep)
{
    if (sleep == 0) {
        sleep = SLEEP_DEFAULT;
    }
    if (isStarted() && sleep == m_sleep) {
        return;
    }
    stop();

    m_pollingSrc = g_timeout_source_new_seconds(sleep);
    g_source_set_callback(m_pollingSrc, (GSourceFunc)&HawkBitClient::poll, this, NULL);
    g_source_attach(m_pollingSrc, g_main_context_default());
    g_source_unref(m_pollingSrc);
    m_sleep = sleep;
}

bool HawkBitClient::isStarted()
{
    return (m_pollingSrc && !g_source_is_destroyed(m_pollingSrc));
}

void HawkBitClient::stop()
{
    if (isStarted()) {
        g_source_destroy(m_pollingSrc);
        m_pollingSrc = nullptr;
    }
}

bool HawkBitClient::getBase(JValue& responsePayload, const string& url)
{
    HttpRequest httpCall;

    if (Setting::getInstance().onLogHttp()) {
        Logger::verbose(getClassName(), "RestAPI", "GET " + url);
    }
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
    if (Setting::getInstance().onLogHttp()) {
        Logger::verbose(getClassName(), "Response - \n" + responsePayload.stringify("    "));
    }
    return true;
}

bool HawkBitClient::getCancellationAction(JValue& requestPayload, JValue& responsePayload, string& id)
{
    const string url = m_hawkBitUrl + "/cancelAction/" + id;
    HttpRequest httpCall;
    httpCall.open(MethodType_GET, url);

    if (Setting::getInstance().onLogHttp()) {
        Logger::verbose(getClassName(), "RestAPI", "GET Cancellation Action");
    }
    return true;
}

bool HawkBitClient::postCancellationAction(JValue& requestPayload, JValue& responsePayload, string& id)
{
    const string url = m_hawkBitUrl + "/cancelAction/" + id + "/feedback";
    HttpRequest httpCall;
    httpCall.open(MethodType_POST, url);

    if (Setting::getInstance().onLogHttp()) {
        Logger::verbose(getClassName(), "RestAPI", "POST Cancellation Action");
    }
    return true;
}

bool HawkBitClient::putConfigData(JValue& requestPayload, JValue& responsePayload)
{
    const string url = m_hawkBitUrl + "/configData";
    HttpRequest httpCall;
    httpCall.open(MethodType_PUT, url);

    if (Setting::getInstance().onLogHttp()) {
        Logger::verbose(getClassName(), "RestAPI", "PUT ConfigData");
    }
    return true;
}

bool HawkBitClient::getDeploymentAction(JValue& requestPayload, JValue& responsePayload, string& id)
{
    const string url = m_hawkBitUrl + "/deploymentBase/" + id;
    HttpRequest httpCall;
    httpCall.open(MethodType_GET, url);

    if (Setting::getInstance().onLogHttp()) {
        Logger::verbose(getClassName(), "RestAPI", "GET Deployment Action");
    }
    return true;
}

bool HawkBitClient::postDeploymentActionSuccess(JValue& responsePayload, const string& id)
{
    const string url = m_hawkBitUrl + "/deploymentBase/" + id + "/feedback";
    if (Setting::getInstance().onLogHttp()) {
        Logger::verbose(getClassName(), "RestAPI", "POST Deployment Action");
    }

    JValue requestPayload = pbnjson::Object();
    requestPayload.put("id", id);
    requestPayload.put("time", Time::getUtcTime());
    requestPayload.put("status", pbnjson::Object());
    requestPayload["status"].put("execution", "closed");
    requestPayload["status"].put("result", pbnjson::Object());
    requestPayload["status"]["result"].put("finished", "success");

    HttpRequest httpCall;
    if (!httpCall.open(MethodType_POST, url) || !httpCall.send(requestPayload)) {
        Logger::error(getClassName(), "Failed to post feedback");
        return false;
    }
    return true;
}

bool HawkBitClient::postDeploymentActionFailed(JValue& responsePayload, const string& id)
{
    const string url = m_hawkBitUrl + "/deploymentBase/" + id + "/feedback";
    if (Setting::getInstance().onLogHttp()) {
        Logger::verbose(getClassName(), "RestAPI", "POST Deployment Action");
    }

    JValue requestPayload = pbnjson::Object();
    requestPayload.put("id", id);
    requestPayload.put("time", Time::getUtcTime());
    requestPayload.put("status", pbnjson::Object());
    requestPayload["status"].put("execution", "closed");
    requestPayload["status"].put("result", pbnjson::Object());
    requestPayload["status"]["result"].put("finished", "failure");

    HttpRequest httpCall;
    if (!httpCall.open(MethodType_POST, url) || !httpCall.send(requestPayload)) {
        Logger::error(getClassName(), "Failed to post feedback");
        return false;
    }
    return true;
}

bool HawkBitClient::getSoftwaremodules(JValue& requestPayload, JValue& responsePayload, string& id)
{
    const string url = m_hawkBitUrl + "/softwaremodules/" + id + "/artifacts";
    HttpRequest httpCall;
    httpCall.open(MethodType_GET, url);

    if (Setting::getInstance().onLogHttp()) {
        Logger::info(getClassName(), "RestAPI", "GET Softwaremodules");
    }
    return true;
}

guint HawkBitClient::poll(gpointer data)
{
    HawkBitClient* self = (HawkBitClient*) data;
    shared_ptr<AbsAction> action;
    JValue responsePayload;
    string sleep = "";
    string href = "";

    Logger::info(self->getClassName(), "== POLLING START ==");
    if (!self->getBase(responsePayload, self->m_hawkBitUrl)) {
        goto Done;
    }

    if (JValueUtil::getValue(responsePayload, "config", "polling", "sleep", sleep)) {
        if (self->m_sleep != Time::toSeconds(sleep)) {
            // TODO
        }
    }

    if (JValueUtil::getValue(responsePayload, "_links", "deploymentBase", "href", href)) {
        if (!self->getBase(responsePayload, href)) {
            goto Done;
        }
        if (self->m_listener)self-> m_listener->onInstallationAction(responsePayload);
    }
    if (JValueUtil::getValue(responsePayload, "_links", "cancelAction", "href", href)) {
        if (!self->getBase(responsePayload, href)) {
            goto Done;
        }
        if (self->m_listener) self->m_listener->onCancellationAction(responsePayload);
    }
    if (JValueUtil::getValue(responsePayload, "_links", "configData", "href", href)) {
        // TODO: Need to be updated
    }

Done:
    Logger::info(self->getClassName(), "== POLLING END ==");
    return G_SOURCE_CONTINUE;
}
