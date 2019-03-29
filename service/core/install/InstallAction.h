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

#ifndef CORE_INSTALL_INSTALLACTION_H_
#define CORE_INSTALL_INSTALLACTION_H_

#include <iostream>
#include <list>
#include <map>

#include "core/AbsAction.h"
#include "core/install/SoftwareModule.h"
#include "interface/IListener.h"

using namespace std;

class InstallAction : public AbsAction {
public:
    InstallAction();
    virtual ~InstallAction();

    // AbsAction
    virtual bool fromJson(const JValue& json) override;

    const string& getDownload()
    {
        return m_download;
    }

    const string& getUpdate()
    {
        return m_update;
    }

    list<shared_ptr<SoftwareModule>>& getSoftwareModules()
    {
        return m_softwareModules;
    }

private:
    string m_download;
    string m_update;

    list<shared_ptr<SoftwareModule>> m_softwareModules;
};

#endif /* CORE_INSTALL_INSTALLACTION_H_ */
