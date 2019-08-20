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

#ifndef LS2_NOTIFICATIONMANAGER_H_
#define LS2_NOTIFICATIONMANAGER_H_

#include <iostream>
#include <pbnjson.hpp>

#include "interface/IInitializable.h"
#include "interface/ISingleton.h"

using namespace pbnjson;
using namespace std;

class NotificationManager : public IInitializable,
                            public ISingleton<NotificationManager> {
friend ISingleton<NotificationManager>;
public:
    virtual ~NotificationManager();

    // IInitializable
    virtual bool onInitialization() override;
    virtual bool onFinalization() override;

    bool createAlert(const string& title, const string& message, JValue buttonArray);
    bool createToast(const string& message);

private:
    NotificationManager();

    static const unsigned long LSCALL_TIMEOUT;
};

#endif /* LS2_NOTIFICATIONMANAGER_H_ */
