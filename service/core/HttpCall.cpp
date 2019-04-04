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

#include "core/HttpCall.h"

#include "external/glibcurl.h"
#include "hardware/AbsHardware.h"
#include "Setting.h"

map<CURL*, HttpCall*> HttpCall::s_httpCallMap = map<CURL*, HttpCall*>();

string HttpCall::toString(long responseCode)
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

HttpCall::HttpCall(const MethodType& methodType, const string& url)
    : m_header(NULL)
    , m_body("")
    , m_payload("")
    , m_size(0)
    , m_file(NULL)
{
    m_curl = curl_easy_init();

    setClassName("HttpCall");
    setMethod(methodType);
    setUrl(url);

    appendHeader("Accept", "application/hal+json");
    appendHeader("Content-Type", "application/json;charset=UTF-8");
    appendHeader("Authorization", "GatewayToken " + AbsHardware::getHardware().getEnv("hawkbit_token"));

    Logger::verbose(getClassName(), "Ready HttpCall for " + (!url.empty() ? url : "null"));
}

HttpCall::~HttpCall()
{
    curl_easy_cleanup(m_curl);
    if (m_header) {
        curl_slist_free_all(m_header);
    }
    close();
}

bool HttpCall::perform()
{
    prepare();

    CURLcode rc = CURLE_OK;
    if (CURLE_OK != (rc = curl_easy_perform(m_curl))) {
        Logger::error(getClassName(), "Failed in curl_easy_perform", curl_easy_strerror(rc));
        return false;
    }

    return true;
}

bool HttpCall::download()
{
    if (m_filename.empty()) {
        Logger::error(getClassName(), "'filename' is empty");
        return false;
    }
    prepare();

    m_file = fopen(m_filename.c_str(), "wb");
    if (m_file == nullptr) {
        Logger::error(getClassName(), "Failed to open file: " + string(strerror(errno)));
        return false;
    }

    glibcurl_set_callback(&HttpCall::onReceiveEvent, nullptr);
    CURLMcode rc = CURLM_OK;
    if (CURLM_OK != (rc = glibcurl_add(m_curl))) {
        Logger::error(getClassName(), "Failed in glibcurl_add", curl_multi_strerror(rc));
        return false;
    }
    s_httpCallMap[m_curl] = this;

    if (m_listener) m_listener->onStartedDownload(this);
    return true;
}

void HttpCall::setUrl(const std::string& url)
{
    CURLcode rc = CURLE_OK;
    if (CURLE_OK != (rc = curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str()))) {
        Logger::error(getClassName(), "Failed in curl_easy_setopt(URL)", curl_easy_strerror(rc));
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
        Logger::error(getClassName(), "Failed in curl_easy_setopt(METHOD)", curl_easy_strerror(rc));
    }
}

void HttpCall::onReceiveEvent(void* userdata)
{
    CURLMsg* curlMsg = nullptr;
    int size;
    while (true) {
        curlMsg = curl_multi_info_read(glibcurl_handle(), &size);
        if (curlMsg == nullptr) {
            break;
        }

        if (curlMsg->msg != CURLMSG_DONE)  {
            Logger::warning("HttpCall", "Unknown CURLMsg");
            continue;
        }

        HttpCall* self = s_httpCallMap[curlMsg->easy_handle];
        if (!self) {
            Logger::error("HttpCall", "HttpCall is null");
            break;
        }

        s_httpCallMap.erase(curlMsg->easy_handle);
        glibcurl_remove(self->m_curl);
        self->close();
        if (self->m_listener) self->m_listener->onCompletedDownload(self);
    }
}

size_t HttpCall::onReceiveData(char* ptr, size_t size, size_t nmemb, void* userdata)
{
    HttpCall* self = static_cast<HttpCall*>(userdata);
    if (!self) {
        Logger::error(self->getClassName(), "userdata is null");
        return 0;
    }

    size_t dataSize;
    if (self->m_file) {
        dataSize = fwrite(ptr, size, nmemb, self->m_file);
    } else {
        dataSize = size * nmemb;
        self->m_payload.append(ptr);
    }
    self->m_size += dataSize;

    if (self->m_listener) self->m_listener->onProgressDownload(self);
    return dataSize;
}

void HttpCall::prepare()
{
    CURLcode rc = CURLE_OK;

    if (Setting::getInstance().getLogCurl()) {
        if (CURLE_OK != (rc = curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L))) {
            Logger::error(getClassName(), "Failed in curl_easy_setopt(VERBOSE)", curl_easy_strerror(rc));
        }
    }

    if (CURLE_OK != (rc = curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, m_header))) {
        Logger::error(getClassName(), "Failed in curl_easy_setopt(HEADER)", curl_easy_strerror(rc));
    }

    if (m_body.length() > 0) {
        if (CURLE_OK != (rc = curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, m_body.length()))) {
            Logger::error(getClassName(), "Failed in curl_easy_setopt(POSTFIELDSIZE)", curl_easy_strerror(rc));
        }

        if (CURLE_OK != (rc = curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, m_body.c_str()))) {
            Logger::error(getClassName(), "Failed in curl_easy_setopt(POSTFIELDS)", curl_easy_strerror(rc));
        }
    }

    if (CURLE_OK != (rc = curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &HttpCall::onReceiveData))) {
        Logger::error(getClassName(), "Failed in curl_easy_setopt(WRITEFUNCTION", curl_easy_strerror(rc));
    }

    if (CURLE_OK != (rc = curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, this))) {
        Logger::error(getClassName(), "Failed in curl_easy_setopt(WRITEDATA)", curl_easy_strerror(rc));
    }
}

void HttpCall::appendHeader(const std::string& key, const std::string& val)
{
    m_header = curl_slist_append(m_header, (key + ": " + val).c_str());
}

void HttpCall::close()
{
    if (m_file) {
        fflush(m_file);
        fclose(m_file);
        m_file = nullptr;
    }
}
