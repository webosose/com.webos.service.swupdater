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

#include <external/glibcurl.h>
#include <hawkbit/HttpCall.h>
#include <Setting.h>
#include "util/Logger.h"

CURL* HttpCall::s_curl = nullptr;

bool HttpCall::initialize()
{
    // Init glibcurl
    glibcurl_init();

    s_curl = curl_easy_init();
    if (s_curl == nullptr)
        return false;
    return true;
}

void HttpCall::finalize()
{
    curl_easy_cleanup(s_curl);
    glibcurl_cleanup();
}

HttpCall::HttpCall(const MethodType& methodType, const string& url, const string& token)
    : m_header(NULL)
    , m_requestPayload("")
    , m_responsePayload("")
    , m_responsePayloadSize(0)
    , m_responseFile(NULL)
{
    setClassName("HttpCall");

    setUrl(url);
    setMethod(methodType);

    appendHeader("Accept", "application/hal+json");
    appendHeader("Content-Type", "application/json;charset=UTF-8");
    appendHeader("Authorization", "GatewayToken " + token);
}

HttpCall::~HttpCall()
{
    if (m_header) {
        curl_slist_free_all(m_header);
    }
}

bool HttpCall::performSync()
{
    preparePerform();

    CURLcode rc = CURLE_OK;
    if (CURLE_OK != (rc = curl_easy_perform(s_curl))) {
        Logger::error(getClassName(), "Failed in curl_easy_perform", curl_easy_strerror(rc));
        return false;
    }

    return true;
}

bool HttpCall::performAsync(HttpCallListener* listener)
{
    preparePerform();
    setListener(listener);
    glibcurl_set_callback(&HttpCall::onGlibcurl, this);

    CURLMcode rc = CURLM_OK;
    if (CURLM_OK != (rc = glibcurl_add(s_curl))) {
        Logger::error(getClassName(), "Failed in glibcurl_add", curl_multi_strerror(rc));
        return false;
    }

    return true;
}

void HttpCall::setUrl(const std::string& url)
{
    CURLcode rc = CURLE_OK;
    if (CURLE_OK != (rc = curl_easy_setopt(s_curl, CURLOPT_URL, url.c_str()))) {
        Logger::error(getClassName(), "Failed in curl_easy_setopt(URL)", curl_easy_strerror(rc));
    }
}

void HttpCall::setMethod(MethodType method)
{
    CURLcode rc = CURLE_OK;
    switch (method) {
    case MethodType_GET:
        rc = curl_easy_setopt(s_curl, CURLOPT_HTTPGET, 1L);
        break;

    case MethodType_POST:
        rc = curl_easy_setopt(s_curl, CURLOPT_POST, 1L);
        break;

    case MethodType_PUT:
        rc = curl_easy_setopt(s_curl, CURLOPT_PUT, 1L);
        break;

    case MethodType_DELETE:
        rc = curl_easy_setopt(s_curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        break;
    }

    if (CURLE_OK != rc) {
        Logger::error(getClassName(), "Failed in curl_easy_setopt(METHOD)", curl_easy_strerror(rc));
    }
}

void HttpCall::setBody(pbnjson::JValue body)
{
    if (!body.isNull())
        m_requestPayload = body.stringify();
}

void HttpCall::preparePerform()
{
    CURLcode rc = CURLE_OK;

    if (Setting::getInstance().getLogCurl()) {
        if (CURLE_OK != (rc = curl_easy_setopt(s_curl, CURLOPT_VERBOSE, 1L))) {
            Logger::error(getClassName(), "Failed in curl_easy_setopt(VERBOSE)", curl_easy_strerror(rc));
        }
    }

    if (CURLE_OK != (rc = curl_easy_setopt(s_curl, CURLOPT_HTTPHEADER, m_header))) {
        Logger::error(getClassName(), "Failed in curl_easy_setopt(HEADER)", curl_easy_strerror(rc));
    }

    if (m_requestPayload.length() > 0) { // or POST
        if (CURLE_OK != (rc = curl_easy_setopt(s_curl, CURLOPT_POSTFIELDSIZE, m_requestPayload.length()))) {
            Logger::error(getClassName(), "Failed in curl_easy_setopt(POSTFIELDSIZE)", curl_easy_strerror(rc));
        }

        if (CURLE_OK != (rc = curl_easy_setopt(s_curl, CURLOPT_POSTFIELDS, m_requestPayload.c_str()))) {
            Logger::error(getClassName(), "Failed in curl_easy_setopt(POSTFIELDS)", curl_easy_strerror(rc));
        }
    }

    if (m_responseFile) {
        if (CURLE_OK != (rc = curl_easy_setopt(s_curl, CURLOPT_WRITEFUNCTION, &HttpCall::onReceiveFile))) {
            Logger::error(getClassName(), "Failed in curl_easy_setopt(WRITEFUNCTION", curl_easy_strerror(rc));
        }

        if (CURLE_OK != (rc = curl_easy_setopt(s_curl, CURLOPT_WRITEDATA, m_responseFile))) {
            Logger::error(getClassName(), "Failed in curl_easy_setopt(WRITEDATA)", curl_easy_strerror(rc));
        }
    } else {
        if (CURLE_OK != (rc = curl_easy_setopt(s_curl, CURLOPT_WRITEFUNCTION, &HttpCall::onReceiveText))) {
            Logger::error(getClassName(), "Failed in curl_easy_setopt(WRITEFUNCTION", curl_easy_strerror(rc));
        }

        if (CURLE_OK != (rc = curl_easy_setopt(s_curl, CURLOPT_WRITEDATA, this))) {
            Logger::error(getClassName(), "Failed in curl_easy_setopt(WRITEDATA)", curl_easy_strerror(rc));
        }
    }
}

long HttpCall::getResponseCode()
{
    long responseCode = 0;

    CURLcode rc = CURLE_OK;
    if (CURLE_OK != (rc = curl_easy_getinfo(s_curl, CURLINFO_RESPONSE_CODE, &responseCode))) {
        Logger::error(getClassName(), "Failed in curl_easy_getinfo(RESPONSE_CODE)", curl_easy_strerror(rc));
    }
    return responseCode;
}

size_t HttpCall::onReceiveText(char* ptr, size_t size, size_t nmemb, void* userdata)
{
    HttpCall* self = static_cast<HttpCall*>(userdata);
    if (!self) {
        Logger::error(self->getClassName(), "data is null", __FUNCTION__);
        return 0;
    }

    size_t responsePayloadSize = size * nmemb;

    self->m_responsePayload.append(ptr);
    self->m_responsePayloadSize += responsePayloadSize;

    return responsePayloadSize;
}

size_t HttpCall::onReceiveFile(void* ptr, size_t size, size_t nmemb, FILE* stream)
{
    size_t bytesRead = fwrite(ptr, size, nmemb, stream);
    return bytesRead;
}

void HttpCall::appendHeader(const std::string& key, const std::string& val)
{
    m_header = curl_slist_append(m_header, (key + ": " + val).c_str());
}

void HttpCall::onGlibcurl(void* data)
{
    HttpCall* self = static_cast<HttpCall*>(data);
    if (!self) {
        Logger::error("HttpCall", "data is null");
        return;
    }

    CURLMsg* msg;
    int inQueue;
    while (true) {
        msg = curl_multi_info_read(glibcurl_handle(), &inQueue);
        if (msg == 0) {
            break;
        }

        if (msg->msg == CURLMSG_DONE)  {
            Logger::info(self->getClassName(), "receive done");
            glibcurl_remove(HttpCall::s_curl);

            if (self->m_listener) {
                self->m_listener->onCompleteHttpCall(*self);
            }
        } else {
            Logger::warning(self->getClassName(), "Unknown CURLMsg code " + (msg->msg));
        }
    }
}
