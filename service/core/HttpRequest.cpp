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

#include "core/HttpRequest.h"

#include "Setting.h"
#include "bootloader/AbsBootloader.h"

string HttpRequest::toString(long responseCode)
{
    switch(responseCode) {
    case 200L:  return "Ok";
    case 400L:  return "Bad Request";
    case 401L:  return "Unauthorized";
    case 403L:  return "Forbidden";
    case 405L:  return "Method Not Allowed";
    case 406L:  return "Not Acceptable";
    case 429L:  return "Too Many Request";
    default:
        break;
    }
    return "Unknown";
}

HttpRequest::HttpRequest()
    : m_type(MethodType_NONE)
    , m_header(NULL)
    , m_requestText("")
    , m_responseText("")
{
    setClassName("HttpCall");

    m_easyHandle = curl_easy_init();

    addHeader("Accept", "application/hal+json");
    addHeader("Content-Type", "application/json;charset=UTF-8");
    addHeader("Authorization", "GatewayToken " + AbsBootloader::getBootloader().getEnv("hawkbit_token"));
}

HttpRequest::~HttpRequest()
{
    if (m_easyHandle)
        curl_easy_cleanup(m_easyHandle);
    if (m_header)
        curl_slist_free_all(m_header);
}

bool HttpRequest::open(const MethodType& methodType, const std::string& url)
{
    if (!setMethod(methodType) || !setUrl(url))
        return false;
    Logger::verbose(getClassName(), __FUNCTION__);
    return true;
}

bool HttpRequest::send(JValue request)
{
    if (m_type == MethodType_NONE || m_easyHandle == nullptr) {
        return false;
    }
    if (!request.isNull() && request.isValid()) {
        m_requestText = request.stringify();
    }
    if (!prepare()) {
        return false;
    }

    CURLcode rc = CURLE_OK;
    rc = curl_easy_setopt(m_easyHandle, CURLOPT_WRITEDATA, this);
    if (rc != CURLE_OK) {
        goto Done;
    }
    rc = curl_easy_setopt(m_easyHandle, CURLOPT_WRITEFUNCTION, &HttpRequest::onReceiveResponse);
    if (rc != CURLE_OK) {
        goto Done;
    }
    rc = curl_easy_perform(m_easyHandle);
    if (rc != CURLE_OK) {
        goto Done;
    }

    Logger::verbose(getClassName(), __FUNCTION__);
    return true;

Done:
    if (rc != CURLE_OK) {
        Logger::error(getClassName(), "Failed in curl_easy_setopt(WRITEFUNCTION", curl_easy_strerror(rc));
        return false;
    }
    return true;
}

size_t HttpRequest::onReceiveResponse(char* ptr, size_t size, size_t nmemb, void* userdata)
{
    HttpRequest* self = static_cast<HttpRequest*>(userdata);
    if (!self) {
        Logger::error(self->getClassName(), "userdata is null");
        return 0;
    }

    size_t dataSize = size * nmemb;
    self->m_responseText.append(ptr);
    return dataSize;
}

bool HttpRequest::prepare()
{
    CURLcode rc = CURLE_OK;
    string errorText;

    if (Logger::getInstance().isVerbose()) {
        rc = curl_easy_setopt(m_easyHandle, CURLOPT_VERBOSE, 1L);
        if (rc != CURLE_OK) {
            goto Done;
        }
    }

    rc = curl_easy_setopt(m_easyHandle, CURLOPT_HTTPHEADER, m_header);
    if (rc != CURLE_OK) {
        goto Done;
    }

    if (m_requestText.length() > 0) {
        rc = curl_easy_setopt(m_easyHandle, CURLOPT_POSTFIELDSIZE, m_requestText.length());
        if (rc != CURLE_OK) {
            goto Done;
        }

        rc = curl_easy_setopt(m_easyHandle, CURLOPT_POSTFIELDS, m_requestText.c_str());
        if (rc != CURLE_OK) {
            goto Done;
        }
    }

Done:
    if (rc != CURLE_OK) {
        Logger::error(getClassName(), "Failed in curl_easy_setopt : ", curl_easy_strerror(rc));
        return false;
    }
    return true;
}

void HttpRequest::addHeader(const std::string& key, const std::string& val)
{
    m_header = curl_slist_append(m_header, (key + ": " + val).c_str());
}

bool HttpRequest::setUrl(const std::string& url)
{
    CURLcode rc = curl_easy_setopt(m_easyHandle, CURLOPT_URL, url.c_str());
    if (rc != CURLE_OK) {
        Logger::error(getClassName(), "Failed in curl_easy_setopt : " + string(curl_easy_strerror(rc)));
        return false;
    }
    m_url = url;
    return true;
}

bool HttpRequest::setMethod(MethodType method)
{
    CURLcode rc = CURLE_OK;
    switch (method) {
    case MethodType_GET:
        rc = curl_easy_setopt(m_easyHandle, CURLOPT_HTTPGET, 1L);
        break;

    case MethodType_POST:
        rc = curl_easy_setopt(m_easyHandle, CURLOPT_POST, 1L);
        break;

    case MethodType_PUT:
        rc = curl_easy_setopt(m_easyHandle, CURLOPT_CUSTOMREQUEST, "PUT");
        break;

    case MethodType_DELETE:
        rc = curl_easy_setopt(m_easyHandle, CURLOPT_CUSTOMREQUEST, "DELETE");
        break;

    default:
        return false;
    }

    if (rc != CURLE_OK) {
        Logger::error(getClassName(), "Failed in curl_easy_setopt : " + string(curl_easy_strerror(rc)));
        return false;
    }

    m_type = method;
    return true;
}
