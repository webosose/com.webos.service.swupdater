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


#ifndef CORE_INSTALL_DESIGN_COMPOSITE_H_
#define CORE_INSTALL_DESIGN_COMPOSITE_H_

#include <iostream>
#include <deque>

#include "interface/ISerializable.h"

using namespace std;

class Composite;

class CompositeListener {
public:
    CompositeListener() {}
    virtual ~CompositeListener() {}

    virtual void onChangedStatus(Composite* composite) = 0;
    virtual void onCompletedDownload(Composite* composite) = 0;
    virtual void onCompletedInstall(Composite* composite) = 0;
    virtual void onFailedDownload(Composite* composite) = 0;
    virtual void onFailedInstall(Composite* composite) = 0;
};

class Composite : public ISerializable {
public:
    Composite();
    virtual ~Composite();

    virtual bool startDownload() = 0;
    virtual bool pauseDownload() = 0;
    virtual bool resumeDownload() = 0;
    virtual bool cancelDownload() = 0;
    virtual bool startInstall() = 0;
    virtual bool cancelInstall() = 0;

    virtual bool toJson(JValue& json) override
    {
        return true;
    }

protected:
    deque<shared_ptr<Composite>> m_children;

    unsigned int m_current;

};

#endif /* CORE_INSTALL_DESIGN_COMPOSITE_H_ */
