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

#ifndef CORE_HTTPREQUEST_H_
#define CORE_HTTPREQUEST_H_

#include <algorithm>
#include <map>
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
    MethodType_NONE,
    MethodType_GET,
    MethodType_POST,
    MethodType_PUT,
    MethodType_DELETE,
};

class HttpRequest : public IClassName {
public:
    static string toString(long responseCode);

    HttpRequest();
    virtual ~HttpRequest();

    virtual bool open(const MethodType& methodType, const std::string& url);
    virtual bool send(JValue request = nullptr);

    long getStatus()
    {
        long responseCode = 0;

        CURLcode rc = curl_easy_getinfo(m_easyHandle, CURLINFO_RESPONSE_CODE, &responseCode);
        if (rc != CURLE_OK) {
            Logger::error(getClassName(), "Failed in curl_easy_getinfo(RESPONSE_CODE)", curl_easy_strerror(rc));
        }
        return responseCode;
    }

    string& getResponseText()
    {
        return m_responseText;
    }

protected:
    static size_t onReceiveResponse(char* contents, size_t size, size_t nmemb, void* userdata);

    bool prepare();
    void addHeader(const std::string& key, const std::string& val);
    bool setUrl(const std::string& url);
    bool setMethod(MethodType method);

    // request
    CURL* m_easyHandle;
    enum MethodType m_type;
    struct curl_slist* m_header;
    string m_url;
    string m_requestText;
    string m_responseText;

};

#endif /* CORE_HTTPREQUEST_H_ */
