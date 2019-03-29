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
#include <external/glibcurl.h>
#include <Setting.h>

CURL* HttpCall::s_curl = nullptr;
string HttpCall::s_token = "";

bool HttpCall::initialize(string& token)
{
    s_token = token;
    glibcurl_init();

    s_curl = curl_easy_init();
    if (s_curl == nullptr)
        return false;
    return true;
}

bool HttpCall::isInitialize()
{
    if (s_token.empty() || s_curl == nullptr)
        return false;
    return true;
}

void HttpCall::finalize()
{
    curl_easy_cleanup(s_curl);
    glibcurl_cleanup();
}

HttpCall::HttpCall(const MethodType& methodType, const string& url)
    : m_header(NULL)
    , m_requestPayload("")
    , m_responsePayload("")
    , m_size(0)
    , m_file(NULL)
{
    setClassName("HttpCall");
    setUrl(url);
    setMethod(methodType);

    appendHeader("Accept", "application/hal+json");
    appendHeader("Content-Type", "application/json;charset=UTF-8");
    appendHeader("Authorization", "GatewayToken " + s_token);

    Logger::verbose(getClassName(), "Ready HttpCall for " + (!url.empty() ? url : "null"));
}

HttpCall::~HttpCall()
{
    if (m_header) {
        curl_slist_free_all(m_header);
    }
    if (m_file) {
        fclose(m_file);
        m_file = nullptr;
    }
}

bool HttpCall::perform()
{
    prepare();

    CURLcode rc = CURLE_OK;
    Logger::verbose(getClassName(), "Perform httpcall");
    if (CURLE_OK != (rc = curl_easy_perform(s_curl))) {
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
    Logger::verbose(getClassName(), "Opened file - " + m_filename);

    glibcurl_set_callback(&HttpCall::onReceiveAsyncEvent, this);

    Logger::verbose(getClassName(), "Download httpcall");
    CURLMcode rc = CURLM_OK;
    if (CURLM_OK != (rc = glibcurl_add(s_curl))) {
        Logger::error(getClassName(), "Failed in glibcurl_add", curl_multi_strerror(rc));
        return false;
    }

    Logger::verbose(getClassName(), "Start download -  " + m_filename);
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

void HttpCall::onReceiveAsyncEvent(void* userdata)
{
    HttpCall* self = static_cast<HttpCall*>(userdata);
    if (!self) {
        Logger::error(self->getClassName(), "userdata is null");
        return;
    }
    Logger::verbose(self->getClassName(), __FUNCTION__);

    CURLMsg* curlMsg = nullptr;
    int size;
    while (true) {
        curlMsg = curl_multi_info_read(glibcurl_handle(), &size);
        if (curlMsg == nullptr) {
            break;
        }

        if (curlMsg->msg != CURLMSG_DONE)  {
            Logger::warning(self->getClassName(), "Unknown CURLMsg");
            continue;
        }

        glibcurl_remove(HttpCall::s_curl);
        Logger::verbose(self->getClassName(), "Complete download - " + self->m_filename);

        if (self->m_listener) {
            self->m_listener->onCompleteDownload(*self);
        }
    }
}

size_t HttpCall::onReceiveData(char* ptr, size_t size, size_t nmemb, void* userdata)
{
    HttpCall* self = static_cast<HttpCall*>(userdata);
    if (!self) {
        Logger::error(self->getClassName(), "userdata is null");
        return 0;
    }
    Logger::verbose(self->getClassName(), __FUNCTION__);

    size_t dataSize;
    if (self->m_file) {
        dataSize = fwrite(ptr, size, nmemb, self->m_file);
        Logger::verbose(self->getClassName(), "Save received data to file - " + to_string(dataSize));
    } else {
        dataSize = size * nmemb;
        self->m_responsePayload.append(ptr);
        Logger::verbose(self->getClassName(), "Save received data to payload - " + to_string(dataSize));
    }
    self->m_size += dataSize;
    return dataSize;
}

void HttpCall::prepare()
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

    if (m_requestPayload.length() > 0) {
        if (CURLE_OK != (rc = curl_easy_setopt(s_curl, CURLOPT_POSTFIELDSIZE, m_requestPayload.length()))) {
            Logger::error(getClassName(), "Failed in curl_easy_setopt(POSTFIELDSIZE)", curl_easy_strerror(rc));
        }

        if (CURLE_OK != (rc = curl_easy_setopt(s_curl, CURLOPT_POSTFIELDS, m_requestPayload.c_str()))) {
            Logger::error(getClassName(), "Failed in curl_easy_setopt(POSTFIELDS)", curl_easy_strerror(rc));
        }
    }

    if (CURLE_OK != (rc = curl_easy_setopt(s_curl, CURLOPT_WRITEFUNCTION, &HttpCall::onReceiveData))) {
        Logger::error(getClassName(), "Failed in curl_easy_setopt(WRITEFUNCTION", curl_easy_strerror(rc));
    }

    if (CURLE_OK != (rc = curl_easy_setopt(s_curl, CURLOPT_WRITEDATA, this))) {
        Logger::error(getClassName(), "Failed in curl_easy_setopt(WRITEDATA)", curl_easy_strerror(rc));
    }
    Logger::verbose(getClassName(), "Ready for transmission");
}

void HttpCall::appendHeader(const std::string& key, const std::string& val)
{
    m_header = curl_slist_append(m_header, (key + ": " + val).c_str());
}
