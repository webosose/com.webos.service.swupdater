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

#include <core/install/AppSoftwareModule.h>
#include "util/Logger.h"
#include "util/JValueUtil.h"
#include "PolicyManager.h"

AppSoftwareModule::AppSoftwareModule()
    : SoftwareModule()
{
    setClassName("ApplicationSoftwareModule");
}

AppSoftwareModule::~AppSoftwareModule()
{
}

bool AppSoftwareModule::startUpdate()
{
    if (m_downloadState.getState() != StateType_COMPLETED) {
        Logger::info(getClassName(), "Downloading is not completed");
        return false;
    }

    for (auto it = m_artifacts.begin(); it != m_artifacts.end(); ++it) {
        if (AppInstaller::getInstance().install(this->getName(), it->getFullname(), this)) {
            return false;
        }
    }
    m_updateState.start();
    return true;
}

void AppSoftwareModule::onInstallSubscription(pbnjson::JValue subscriptionPayload)
{
    string state;

    if (!JValueUtil::getValue(subscriptionPayload, "details", "state", state))
        return;

    if (state == "installing") {
        m_updateState.start();
    } else if (state == "installed") {
        m_updateState.complete();
        getCall().cancel();
    }
    PolicyManager::getInstance().onChangeStatus();
}
