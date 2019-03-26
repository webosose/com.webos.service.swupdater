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

#include "PolicyManager.h"

#include "core/Action.h"
#include "core/Feedback.h"
#include "util/Logger.h"

PolicyManager::PolicyManager()
{
    setClassName("PolicyManager");
}

PolicyManager::~PolicyManager()
{
}

bool PolicyManager::onInitialization()
{
    HawkBitClient::getInstance().setListener(this);
    LS2Handler::getInstance().setListener(this);

    // first polling
    HawkBitClient::getInstance().poll(&HawkBitClient::getInstance());
    return true;
}

bool PolicyManager::onFinalization()
{
    return true;
}

bool PolicyManager::onCheck(JValue& responsePayload/**/)
{
    // HawkBitClient::getInstance().poll();
    // TODO API에 해당하는 인자전달이 필요함.
    return true;
}

bool PolicyManager::onInstall(JValue& responsePayload/**/)
{
    return true;
}

bool PolicyManager::onCancel(JValue& responsePayload/**/)
{
    return true;
}

bool PolicyManager::onGetStatus(JValue& responsePayload/**/)
{
    return true;
}

void PolicyManager::onCancelUpdate(Action& action)
{
//    JValue json = Object();
//    action->toJson(json);
//    Logger::verbose(getClassName(), "CancelAction", "\n" + json.stringify("  "));

    // cancel

    // feedback
}

void PolicyManager::onInstallUpdate(ActionInstall& _action)
{
//    shared_ptr<ActionInstall> action = dynamic_pointer_cast<ActionInstall>(_action);
//    JValue json = Object();
//    action->toJson(json);
//    Logger::verbose(getClassName(), "InstallAction", "\n" + json.stringify("  "));

    // install

    // feedback

}
