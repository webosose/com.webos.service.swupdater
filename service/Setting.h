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

using namespace std;

class Setting : public IInitializable, public ISingleton<Setting> {
friend class ISingleton<Setting>;
public:
    virtual ~Setting();

    void printHelp();

    virtual bool onInitialization() override;
    virtual bool onFinalization() override;

    virtual string& getId()
    {
        return m_id;
    }

private:
    Setting();

    string m_id;
};

#endif /* SETTING_H_ */
