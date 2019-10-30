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

#ifndef CORE_INSTALL_IMPL_ARTIFACTLEAF_H_
#define CORE_INSTALL_IMPL_ARTIFACTLEAF_H_

#include <iostream>
#include <pbnjson.hpp>

#include "core/Status.h"
#include "core/HttpFile.h"
#include "core/install/design/Composite.h"
#include "ls2/AppInstaller.h"
#include "interface/IClassName.h"
#include "interface/IListener.h"
#include "interface/ISerializable.h"

using namespace std;
using namespace pbnjson;

class ArtifactLeaf;

class ArtifactLeafListener {
public:
    ArtifactLeafListener() {}
    virtual ~ArtifactLeafListener() {}

    virtual void onChangedStatus(ArtifactLeaf* artifact) = 0;
    virtual void onCompletedDownload(ArtifactLeaf* artifact) = 0;
    virtual void onCompletedInstall(ArtifactLeaf* artifact) = 0;
    virtual void onFailedDownload(ArtifactLeaf* artifact) = 0;
    virtual void onFailedInstall(ArtifactLeaf* artifact) = 0;
};

class ArtifactLeaf : public IClassName,
                     public HttpFileListener,
                     public AppInstallerListener,
                     public Composite,
                     public IListener<ArtifactLeafListener> {
public:
    ArtifactLeaf();
    virtual ~ArtifactLeaf();

    // HttpFileListener
    virtual void onStartedDownload(HttpFile* call) override;
    virtual void onProgressDownload(HttpFile* call) override;
    virtual void onCompletedDownload(HttpFile* call) override;
    virtual void onFailedDownload(HttpFile* call) override;

    // AppInstallerListener
    virtual void onInstallSubscription(pbnjson::JValue subscriptionPayload) override;

    // Composite
    virtual bool startDownload() override;
    virtual bool pauseDownload() override;
    virtual bool resumeDownload() override;
    virtual bool cancelDownload() override;
    virtual bool startInstall() override;
    virtual bool cancelInstall() override;

    // ISerializable
    virtual bool fromJson(const JValue& json) override;
    virtual bool toJson(JValue& json) override;

    void setMetadata(JValue metadata)
    {
        m_metadata = metadata.duplicate();
    }

    const string& getFileName()
    {
        return m_fileName;
    }

    const string getFileExtension()
    {
        string extension = m_fileName.substr(m_fileName.find_last_of(".") + 1);
        return extension;
    }

    const string getIpkName()
    {
        string ipkName = m_fileName.substr(0, m_fileName.find_first_of("_"));
        return ipkName;
    }

    const string getDownloadName()
    {
        return DIRNAME + m_fileName.substr(0, m_fileName.find_last_of(".")) + "." + m_sha1 + "." + getFileExtension();
    }

private:
    const static string DIRNAME;

    // file info
    string m_fileName;

    // file size
    int m_total;
    int m_curSize;
    int m_prevSize;

    // hash value
    string m_sha1;
    string m_md5;

    // download link
    string m_md5sum;
    string m_url;

    shared_ptr<HttpFile> m_httpFile;
    JValue m_metadata;
};

#endif /* CORE_INSTALL_IMPL_ARTIFACTLEAF_H_ */
