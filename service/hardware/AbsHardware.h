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

#ifndef HARDWARE_ABSHARDWARE_H_
#define HARDWARE_ABSHARDWARE_H_

#include <iostream>

#include "interface/IManageable.h"

using namespace std;

class AbsHardware {
friend IManageable<AbsHardware>;
public:
    static AbsHardware& getHardware();

    AbsHardware();
    virtual ~AbsHardware();

    virtual void setEnv(const string& key, const string& value) = 0;
    virtual string getEnv(const string& key) = 0;

};

#endif /* HARDWARE_ABSHARDWARE_H_ */
