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

#ifndef LS2_SYSTEMSERVICE_H_
#define LS2_SYSTEMSERVICE_H_

#include <iostream>
#include <pbnjson.hpp>
#include <luna-service2/lunaservice.hpp>

#include "interface/IClassName.h"
#include "interface/ISingleton.h"

using namespace std;
using namespace pbnjson;

class SystemService : public IClassName,
                      public ISingleton<SystemService> {
friend ISingleton<SystemService>;
public:
    virtual ~SystemService();

    bool queryOSInfo(JValue& responsePayload);
    bool queryDeviceInfo(JValue& responsePayload);

private:
    SystemService();

    static const string SERVICE_NAME;
    static const unsigned long LSCALL_TIMEOUT;

};

#endif /* LS2_SYSTEMSERVICE_H_ */
