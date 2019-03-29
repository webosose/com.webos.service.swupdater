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

#include "core/HttpCall.h"
#include "interface/IClassName.h"
#include "interface/IListener.h"
#include "interface/ISerializable.h"

using namespace std;
using namespace pbnjson;

class Artifact;

class ArtifactListener {
public:
    ArtifactListener() {};
    virtual ~ArtifactListener() {};

    virtual void onSuccessDownload(Artifact& actifact) = 0;
    virtual void onProgressDownload(Artifact& actifact) = 0;
    virtual void onErrorDownload(Artifact& actifact) = 0;

};

class Artifact : public IClassName,
                 public IListener<ArtifactListener>,
                 public ISerializable,
                 public HttpCallListener {
public:
    Artifact(const JValue& json);
    virtual ~Artifact();

    bool download();
    void onCompleteDownload(HttpCall& call) override;

    const int getSize()
    {
        return m_size;
    }

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

    const string& getMd5Sum()
    {
        return m_md5sum;
    }

    const string& getDownload() const
    {
        return m_download;
    }

private:
    // ISerializable
    virtual bool fromJson(const JValue& json) override;

    int m_size;
    string m_filename;

    // hash value
    string m_sha1;
    string m_md5;

    // download link
    string m_md5sum;
    string m_download;

    shared_ptr<HttpCall> m_httpCall;
};

#endif /* CORE_INSTALL_ARTIFACT_H_ */
