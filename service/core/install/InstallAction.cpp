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

#include <core/install/InstallAction.h>

InstallAction::InstallAction()
    : AbsAction()
    , m_download("")
    , m_update("")
{
    setType(ActionType_INSTALL);
}

InstallAction::~InstallAction()
{
}

bool InstallAction::fromJson(const JValue& json)
{
    AbsAction::fromJson(json);

    if (!json.hasKey("deployment")) {
        return false;
    }
    JValue deployment = json["deployment"];

    m_download = deployment["download"].asString();
    m_update = deployment["update"].asString();

    if (deployment.hasKey("chunks") && deployment["chunks"].isArray()) {
        for (JValue chunk : deployment["chunks"].items()) {
            shared_ptr<SoftwareModule> softwareModule = SoftwareModule::createSoftwareModule(chunk);
            if (softwareModule) {
                m_softwareModules.push_back(softwareModule);
            }
        }
    }
    return true;
}
