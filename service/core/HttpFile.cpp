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

#include "core/HttpFile.h"
#include "external/glibcurl.h"
#include "Setting.h"

map<CURL*, HttpFile*> HttpFile::s_map;

HttpFile::HttpFile()
    : HttpRequest()
    , m_filename("")
    , m_file(nullptr)
    , m_size(0)
{
    s_map[m_easyHandle] = this;
}

HttpFile::~HttpFile()
{
    s_map.erase(m_easyHandle);
    close();
}

bool HttpFile::send(JValue request)
{
    CURLcode rc1;
    CURLMcode rc2;

    if (m_filename.empty()) {
        Logger::error(getClassName(), "'filename' is empty");
        return false;
    }
    // m_file = fopen(m_filename.c_str(), "ab");
    m_file = fopen(m_filename.c_str(), "wb");
    if (m_file == nullptr) {
        Logger::error(getClassName(), "Failed to open file : " + string(strerror(errno)));
        return false;
    }
    Logger::verbose(getClassName(), "Open file - " + m_filename);
    if (!prepare()) {
        return false;
    }
    /* Following code should be uncommented after implementing pause/resume operations.
    long position = ftell(m_file);
    if (position > 0) {
        addHeader("Range", "bytes=" + to_string(position) + "-");
        m_size = position;
    }*/
    rc1 = curl_easy_setopt(m_easyHandle, CURLOPT_WRITEDATA, this);
    if (rc1 != CURLE_OK) {
        goto Done;
    }
    rc1 = curl_easy_setopt(m_easyHandle, CURLOPT_WRITEFUNCTION, &HttpFile::onReceiveFileData);
    if (rc1 != CURLE_OK) {
        goto Done;
    }
    glibcurl_set_callback(&HttpFile::onReceiveFileEvent, nullptr);
    rc2 = glibcurl_add(m_easyHandle);

Done:
    if (rc1 != CURLE_OK) {
        Logger::error(getClassName(), "Failed in curl_easy_setopt", curl_easy_strerror(rc1));
        return false;
    }
    if (rc2 != CURLM_OK) {
        Logger::error(getClassName(), "Failed in glibcurl_add", curl_multi_strerror(rc2));
        return false;
    }
    Logger::verbose(getClassName(), "Downloading is started. Try to call 'onStartedDownload'");
    m_size = 0;
    if (m_listener) {
        m_listener->onStartedDownload(this);
    }
    return true;
}

void HttpFile::onReceiveFileEvent(void* userdata)
{
    while (true) {
        int size;
        CURLMsg* curlMsg = curl_multi_info_read(glibcurl_handle(), &size);
        if (curlMsg == nullptr) {
            break;
        }

        if (curlMsg->msg != CURLMSG_DONE)  {
            Logger::warning("HttpFile", "Unknown CURLMsg");
            continue;
        }

        HttpFile* self = s_map[curlMsg->easy_handle];
        if (!self) {
            Logger::error("HttpFile", "HttpCall is null");
            break;
        }

        glibcurl_remove(self->m_easyHandle);

        Logger::verbose("HttpFile", "Downloading is completed. Try to call 'onCompletedDownload'");
        self->close();
        if (self->m_listener) {
            self->m_listener->onCompletedDownload(self);
        }
    }
}

size_t HttpFile::onReceiveFileData(char* ptr, size_t size, size_t nmemb, void* userdata)
{
    HttpFile* self = (HttpFile*)userdata;
    if (!self) {
        Logger::error(self->getClassName(), "userdata is null");
        return 0;
    }

    size_t dataSize = fwrite(ptr, size, nmemb, self->m_file);
    self->m_size += dataSize;

    if (self->m_listener) {
        self->m_listener->onProgressDownload(self);
    }
    return dataSize;
}

void HttpFile::close()
{
    if (m_file) {
        fflush(m_file);
        fclose(m_file);
        m_filename = "";
        m_file = nullptr;
    }
}
