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
#include "interface/IInstallable.h"
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

class HttpCall : public IClassName,
                 public IInstallable {
public:
    static string toString(long responseCode);

    HttpCall(const MethodType& methodType, const string& url);
    virtual ~HttpCall();

    bool perform();

    // IInstallable
    virtual bool onStartDownloading() override;

    void setUrl(const std::string& url);
    void setMethod(MethodType method);
    void setBody(JValue& body)
    {
        if (!body.isNull())
            m_requestPayload = body.stringify();
    }

    void setFilename(const string& filename)
    {
        m_filename = filename;
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
        return m_responsePayload;
    }

    size_t getResponseSize()
    {
        return m_size;
    }

    void setResponseFile(FILE* fp)
    {
        m_file = fp;
    }

    FILE* getResponseFile()
    {
        return m_file;
    }

private:
    static void onReceiveAsyncEvent(void* userdata);
    static size_t onReceiveData(char* contents, size_t size, size_t nmemb, void* userdata);

    void prepare();
    void appendHeader(const std::string& key, const std::string& val);

    CURL* m_curl;
    MethodType m_methodType;
    string m_url;
    struct curl_slist* m_header;
    string m_requestPayload;

    string m_responsePayload;
    size_t m_size;

    string m_filename;
    FILE* m_file;

};

#endif /* CORE_HTTPCALL_H_ */
