// Copyright (c) 2020 LG Electronics, Inc.
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

#ifndef UPDATER_BLOCK_BLOCKUPDATER_H_
#define UPDATER_BLOCK_BLOCKUPDATER_H_

#include <iostream>

#include "updater/AbsUpdater.h"

using namespace std;

class BlockUpdater : public AbsUpdater {
friend class AbsUpdaterFactory;
public:
    virtual ~BlockUpdater();

    virtual bool onInitialization() override;
    virtual bool onFinalization() override;

    virtual bool deploy(const string& path, PartitionLabel partitionLabel) override;
    virtual bool undeploy() override;
    virtual bool setReadWriteMode() override;
    virtual bool isUpdated() override;
    virtual void printDebug() override;

private:
    BlockUpdater();
};

#endif /* UPDATER_BLOCK_BLOCKUPDATER_H_ */
