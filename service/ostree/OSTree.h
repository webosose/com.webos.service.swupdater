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

#ifndef OSTREE_OSTREE_H_
#define OSTREE_OSTREE_H_

#include <iostream>
#include <ostree-1/ostree.h>

#include "interface/IClassName.h"

using namespace std;

class OSTree : public IClassName {
public:
    static bool deployDelta(const string& path);
    static void printDebug();

private:
    OSTree(bool writeLock);
    virtual ~OSTree();

    bool m_isInitialized;
    OstreeSysroot* m_sysroot;

};

#endif /* OSTREE_OSTREE_H_ */
