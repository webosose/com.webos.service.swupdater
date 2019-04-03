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

#include <core/State.h>
#include <iostream>
#include <pbnjson.hpp>

#include "core/HttpCall.h"
#include "interface/IClassName.h"
#include "interface/IListener.h"
#include "interface/ISerializable.h"

using namespace std;
using namespace pbnjson;

class Artifact;

class Artifact : public IClassName,
                 public State,
                 public HttpCallListener,
                 public ISerializable {
public:
    Artifact(const JValue& json);
    virtual ~Artifact();

    // IInstallable
    virtual bool ready() override;
    virtual bool start() override;

    // HttpCallListener
    virtual void onStartedDownload(HttpCall* call) override;
    virtual void onProgressDownload(HttpCall* call) override;
    virtual void onCompletedDownload(HttpCall* call) override;
    virtual void onFailedDownload(HttpCall* call) override;

    // ISerializable
    virtual bool fromJson(const JValue& json) override;
    virtual bool toJson(JValue& json) override;

    // getter
    const string& getFullname()
    {
        return m_fullname;
    }

private:
    string m_filename;
    string m_fullname;
    int m_total;
    int m_size;

    // hash value
    string m_sha1;
    string m_md5;

    // download link
    string m_md5sum;
    string m_download;

    shared_ptr<HttpCall> m_httpCall;
};

#endif /* CORE_INSTALL_ARTIFACT_H_ */
