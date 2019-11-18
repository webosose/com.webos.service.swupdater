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

#ifndef SETTING_H_
#define SETTING_H_

#include <iostream>
#include <string>

#include "interface/IInitializable.h"
#include "interface/ISingleton.h"

#ifdef INCLUDE_WEBOS
#include "Environment.h"
#else
#define NAME_SWUPDATER                      "com.webos.service.swupdater"

#define HAWKBIT_ADDRESS                     "http://10.178.84.116:8080"
#define HAWKBIT_TOKEN                       "377b83e10b9f894883e98351875151cb"
#define HAWKBIT_TENANT                      "DEFAULT"

#define PATH_PREFERENCE                     "/var/preferences/" NAME_SWUPDATER
#define FILE_HAWKBIT_INFO                   "hawkBitInfo.json"
#endif

using namespace std;

class Setting : public IInitializable, public ISingleton<Setting> {
friend class ISingleton<Setting>;
public:
    virtual ~Setting();

    void printHelp();

    virtual bool onInitialization() override;
    virtual bool onFinalization() override;

private:
    Setting();
};

#endif /* SETTING_H_ */
