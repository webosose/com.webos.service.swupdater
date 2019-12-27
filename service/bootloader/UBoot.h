// Copyright (c) 2019-2020 LG Electronics, Inc.
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

#ifndef BOOTLOADER_UBOOT_H_
#define BOOTLOADER_UBOOT_H_

#include <iostream>
#include <sstream>

#include "bootloader/AbsBootloader.h"

using namespace std;

class UBoot : public AbsBootloader {
public:
    UBoot();
    virtual ~UBoot();

    virtual void setEnv(const string& key, const string& value) override;
    virtual string getEnv(const string& key) override;

    virtual void notifyUpdate() override;
    virtual void setBootSuccess() override;

    virtual int getBootSlot() override
    {
        return 0;
    }

    virtual int setActive(int slot) override
    {
        return 0;
    }

};

#endif /* BOOTLOADER_UBOOT_H_ */
