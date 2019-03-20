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
#include <curl/curl.h>
#include <pbnjson.hpp>
#include <sstream>
#include <string>

using namespace pbnjson;
using namespace std;

class HttpCall {
public:
    enum MethodType {
        MethodType_GET,
        MethodType_POST,
        MethodType_PUT,
        MethodType_DELETE,
    };

    HttpCall(const string& url, MethodType methodType);
    virtual ~HttpCall();

    void setUrl(const std::string& url);
    void setMethod(MethodType method);
    void appendHeader(const std::string& key, const std::string& val);
    void setBody(std::string& body);
    void setBody(pbnjson::JValue body);
    bool perform();
    long getResponseCode();
    stringstream& getResponse();

public:
    static const string LOG_PREFIX;

    static size_t onReceiveBody(char* contents, size_t size, size_t nmemb, void* userdata);

    CURL *m_curl;
    string m_url;
    MethodType m_methodType;
    struct curl_slist* m_header;
    string m_body;
    stringstream m_response;
};

#endif /* CORE_HTTPCALL_H_ */
