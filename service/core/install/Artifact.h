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

#ifndef CORE_INSTALL_ARTIFACT_H_
#define CORE_INSTALL_ARTIFACT_H_

#include <iostream>
#include <pbnjson.hpp>

#include "interface/ISerializable.h"

using namespace std;
using namespace pbnjson;

class Artifact : public ISerializable {
public:
    Artifact();
    virtual ~Artifact();

    const string& getFilename() const
    {
        return m_filename;
    }

    const string& getSha1()
    {
        return m_sha1;
    }

    const string& getMd5()
    {
        return m_md5;
    }

    int getSize()
    {
        return m_size;
    }

    const string& getDownloadHttps()
    {
        return m_downloadHttps;
    }

    const string& getDownloadHttp() const
    {
        return m_downloadHttp;
    }

    const string& getMd5sumHttps()
    {
        return m_md5sumHttps;
    }

    const string& getMd5sumHttp()
    {
        return m_md5sumHttp;
    }

    virtual bool fromJson(const JValue& json) override;

private:
    string m_filename;
    string m_sha1;
    string m_md5;

    int m_size;

    string m_downloadHttps;
    string m_md5sumHttps;
    string m_downloadHttp;
    string m_md5sumHttp;
};

#endif /* CORE_INSTALL_ARTIFACT_H_ */
