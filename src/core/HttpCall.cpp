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

#include <core/HttpCall.h>
#include "util/Logger.h"

const string HttpCall::LOG_PREFIX = "HttpCall";

HttpCall::HttpCall(const string& url, MethodType methodType)
    : m_curl(curl_easy_init())
    , m_header(NULL)
{
    if (!m_curl) {
        Logger::error("HttpRequest", "Failed in curl_easy_init");
    }

    setUrl(url);
    setMethod(methodType);
}

HttpCall::~HttpCall()
{
    if (m_header) {
        curl_slist_free_all(m_header);
    }

    if (m_curl) {
        curl_easy_cleanup(m_curl);
    }
}


void HttpCall::setUrl(const std::string& url)
{
    CURLcode rc = CURLE_OK;
    if (CURLE_OK != (rc = curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str()))) {
        Logger::error(LOG_PREFIX, "Failed in curl_easy_setopt(URL)", curl_easy_strerror(rc));
    }
}

void HttpCall::setMethod(MethodType method)
{
    CURLcode rc = CURLE_OK;
    switch (method) {
    case MethodType_GET:
        rc = curl_easy_setopt(m_curl, CURLOPT_HTTPGET, 1L);
        break;

    case MethodType_POST:
        rc = curl_easy_setopt(m_curl, CURLOPT_POST, 1L);
        break;

    case MethodType_PUT:
        rc = curl_easy_setopt(m_curl, CURLOPT_PUT, 1L);
        break;

    case MethodType_DELETE:
        rc = curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        break;
    }

    if (CURLE_OK != rc) {
        Logger::error(LOG_PREFIX, "Failed in curl_easy_setopt(METHOD)", curl_easy_strerror(rc));
    }
}

void HttpCall::appendHeader(const std::string& key, const std::string& val)
{
    m_header = curl_slist_append(m_header, (key + ": " + val).c_str());
}

void HttpCall::setBody(std::string& body)
{
    m_body = body;
}

void HttpCall::setBody(pbnjson::JValue body)
{
    string json = body.stringify();

    setBody(json);
}

bool HttpCall::perform()
{
    CURLcode rc = CURLE_OK;
    if (CURLE_OK != (rc = curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L))) {
        Logger::error(LOG_PREFIX, "Failed in curl_easy_setopt(VERBOSE)", curl_easy_strerror(rc));
    }

    if (CURLE_OK != (rc = curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, m_header))) {
        Logger::error(LOG_PREFIX, "Failed in curl_easy_setopt(HEADER)", curl_easy_strerror(rc));
    }

    if (m_body.length() > 0) { // or POST
        if (CURLE_OK != (rc = curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, m_body.length()))) {
            Logger::error(LOG_PREFIX, "Failed in curl_easy_setopt(POSTFIELDSIZE)", curl_easy_strerror(rc));
        }

        if (CURLE_OK != (rc = curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, m_body.c_str()))) {
            Logger::error(LOG_PREFIX, "Failed in curl_easy_setopt(POSTFIELDS)", curl_easy_strerror(rc));
        }
    }

    if (CURLE_OK != (rc = curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &HttpCall::onReceiveBody))) {
        Logger::error(LOG_PREFIX, "Failed in curl_easy_setopt(WRITEFUNCTION", curl_easy_strerror(rc));
    }

    if (CURLE_OK != (rc = curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, this))) {
        Logger::error(LOG_PREFIX, "Failed in curl_easy_setopt(WRITEDATA)", curl_easy_strerror(rc));
    }

    if (CURLE_OK != (rc = curl_easy_perform(m_curl))) {
        Logger::error(LOG_PREFIX, "Failed in curl_easy_perform", curl_easy_strerror(rc));
        return false;
    }

    return true;
}

size_t HttpCall::onReceiveBody(char* contents, size_t size, size_t nmemb, void* data)
{
    HttpCall* self = static_cast<HttpCall*>(data);
    if (!self) {
        Logger::error(LOG_PREFIX, "data is null", __FUNCTION__);
        return 0;
    }

    size_t bodySize = size * nmemb;
    self->m_response << contents;

    return bodySize;
}

long HttpCall::getResponseCode()
{
    long code = 0;

    CURLcode rc = CURLE_OK;
    if (CURLE_OK != (rc = curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &code))) {
        Logger::error(LOG_PREFIX, "Failed in curl_easy_getinfo(RESPONSE_CODE)", curl_easy_strerror(rc));
    }
    return code;
}

std::stringstream& HttpCall::getResponse()
{
    return m_response;
}

