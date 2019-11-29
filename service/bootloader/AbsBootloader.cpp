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

#include "bootloader/AbsBootloader.h"
#include "bootloader/SA8155.h"
#include "bootloader/UBoot.h"
#include "Setting.h"
#include "util/Socket.h"

AbsBootloader& AbsBootloader::getBootloader()
{
#if defined(UBOOT)
    static UBoot instance;
#else
    static SA8155 instance;
#endif
    return instance;
}

AbsBootloader::AbsBootloader()
    : IClassName()
{
    setClassName("AbsBootloader");
}

AbsBootloader::~AbsBootloader()
{
}
