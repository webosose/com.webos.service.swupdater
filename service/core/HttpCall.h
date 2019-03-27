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

#include "interface/IClassName.h"

using namespace pbnjson;
using namespace std;

enum MethodType {
    MethodType_GET,
    MethodType_POST,
    MethodType_PUT,
    MethodType_DELETE,
};

class HttpCall : public IClassName {
public:
    typedef function<void (HttpCall*)> AsyncCallback;

    static bool initialize();
    static void finalize();
    static void cbGlibcurl(void* data);

    HttpCall(const MethodType& methodType, const string& url, const string& token);
    virtual ~HttpCall();

    void setUrl(const std::string& url);
    void setMethod(MethodType method);
    void setBody(pbnjson::JValue body);

    bool performSync();
    bool performAsync(AsyncCallback callback);

    long getResponseCode();

    string& getResponsePayload()
    {
        return m_responsePayload;
    }

    size_t getResponsePayloadSize()
    {
        return m_responsePayloadSize;
    }

    void setResponseFile(FILE* fp)
    {
        m_responseFile = fp;
    }

    FILE* getResponseFile()
    {
        return m_responseFile;
    }

private:
    static size_t onReceiveText(char* contents, size_t size, size_t nmemb, void* userdata);
    static size_t onReceiveFile(void* ptr, size_t size, size_t nmemb, FILE* stream);

    void appendHeader(const std::string& key, const std::string& val);
    void preparePerform();

    static CURL* s_curl;

    MethodType m_methodType;
    string m_url;
    struct curl_slist* m_header;

    string m_requestPayload;
    string m_responsePayload;
    size_t m_responsePayloadSize;
    FILE* m_responseFile;

    AsyncCallback m_asyncCallback;
};

#endif /* CORE_HTTPCALL_H_ */
