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

#ifndef CORE_HTTPCALL_H_
#define CORE_HTTPCALL_H_

#include <algorithm>
#include <sstream>
#include <string>
#include <curl/curl.h>
#include <pbnjson.hpp>

#include "interface/IClassName.h"
#include "interface/IListener.h"
#include "util/Logger.h"

using namespace pbnjson;
using namespace std;

enum MethodType {
    MethodType_GET,
    MethodType_POST,
    MethodType_PUT,
    MethodType_DELETE,
};

class HttpCall;

class HttpCallListener {
public:
    HttpCallListener() {}
    virtual ~HttpCallListener() {}

    virtual void onStartedDownload(HttpCall* call) = 0;
    virtual void onProgressDownload(HttpCall* call) = 0;
    virtual void onCompletedDownload(HttpCall* call) = 0;
    virtual void onFailedDownload(HttpCall* call) = 0;

};

class HttpCall : public IClassName,
                 public IListener<HttpCallListener> {
public:
    static string toString(long responseCode);

    HttpCall(const MethodType& methodType, const string& url);
    virtual ~HttpCall();

    bool perform();
    bool download();

    void setUrl(const std::string& url);
    void setMethod(MethodType method);
    void setBody(JValue& body)
    {
        if (!body.isNull())
            m_body = body.stringify();
    }

    void setFilename(const string& filename)
    {
        m_filename = filename;
    }

    const string& getFilename()
    {
        return m_filename;
    }

    long getResponseCode()
    {
        long responseCode = 0;

        CURLcode rc = curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &responseCode);
        if (rc != CURLE_OK) {
            Logger::error(getClassName(), "Failed in curl_easy_getinfo(RESPONSE_CODE)", curl_easy_strerror(rc));
        }
        return responseCode;
    }

    string& getResponsePayload()
    {
        return m_payload;
    }

    size_t getResponseSize()
    {
        return m_size;
    }

private:
    static void onReceiveAsyncEvent(void* userdata);
    static size_t onReceiveData(char* contents, size_t size, size_t nmemb, void* userdata);

    void prepare();
    void appendHeader(const std::string& key, const std::string& val);

    // request
    CURL* m_curl;
    MethodType m_methodType;
    string m_url;
    struct curl_slist* m_header;
    string m_body;

    // response
    string m_payload;
    string m_filename;
    size_t m_size;
    FILE* m_file;

};

#endif /* CORE_HTTPCALL_H_ */
