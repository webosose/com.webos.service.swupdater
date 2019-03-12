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

#ifndef _MANAGER_UPDATEMANAGER_H_
#define _MANAGER_UPDATEMANAGER_H_

#include <luna-service2/lunaservice.hpp>
#include <pbnjson.hpp>
#include <util/Socket.h>
#include "interface/IManageable.h"

using namespace std;
using namespace pbnjson;

class UpdateManager : public IManageable<UpdateManager>, public LS::Handle {
friend IManageable<UpdateManager>;
public:
    virtual ~UpdateManager();

    virtual bool onInitialization() override;
    virtual bool onFinalization() override;

    // API
    bool status(LSMessage &message);
    void postStatus(JValue& responsePayload);

private:
    static const string NAME;

    UpdateManager();

    LS::SubscriptionPoint m_statusSubscription;
};

#endif /* _MANAGER_UPDATEMANAGER_H_ */
