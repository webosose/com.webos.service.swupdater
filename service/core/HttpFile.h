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

#ifndef CORE_HTTPFILE_H_
#define CORE_HTTPFILE_H_

#include <iostream>

#include "core/HttpRequest.h"

using namespace std;

class HttpFile;

class HttpFileListener {
public:
    HttpFileListener() {}
    virtual ~HttpFileListener() {}

    virtual void onStartedDownload(HttpFile* call) = 0;
    virtual void onProgressDownload(HttpFile* call) = 0;
    virtual void onCompletedDownload(HttpFile* call) = 0;
    virtual void onFailedDownload(HttpFile* call) = 0;

};

class HttpFile : public HttpRequest,
                 public IListener<HttpFileListener> {
public:
    HttpFile();
    virtual ~HttpFile();

    virtual bool send(JValue request = nullptr) override;

    void setFilename(const string& filename)
    {
        m_filename = filename;
    }

    const string& getFilename()
    {
        return m_filename;
    }

    size_t getFilesize()
    {
        return m_size;
    }

private:
    static size_t onReceiveFileData(char* ptr, size_t size, size_t nmemb, void* userdata);
    static void onReceiveFileEvent(void* userdata);

    void close();

    static map<CURL*, HttpFile*> s_map;

    string m_filename;
    FILE* m_file;
    size_t m_size;
};

#endif /* CORE_HTTPFILE_H_ */
