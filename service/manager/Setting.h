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

#ifndef MANAGER_SETTING_H_
#define MANAGER_SETTING_H_

#include <iostream>
#include <string>

#include "interface/IManageable.h"

using namespace std;

class Setting : public IManageable<Setting> {
friend class IManageable<Setting>;
public:
    virtual ~Setting();

    virtual bool onInitialization() override;
    virtual bool onFinalization() override;

    virtual bool getLogCurl()
    {
        return m_logCurl;
    }

    virtual bool verbose()
    {
        return m_verbose;
    }

private:
    Setting();

    bool m_logCurl;
    bool m_verbose;
};

#endif /* MANAGER_SETTING_H_ */
