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

#ifndef UPDATER_ABSUPDATER_H_
#define UPDATER_ABSUPDATER_H_

#include <iostream>

#include "interface/IInitializable.h"
#include "interface/ISingleton.h"

using namespace std;

class AbsUpdater : public IInitializable,
                   public ISingleton<AbsUpdater> {
friend ISingleton<AbsUpdater>;
public:
    virtual ~AbsUpdater() {}

    virtual bool deploy(const string& path) = 0;
    virtual bool undeploy() = 0;
    virtual bool setReadWriteMode() = 0;
    virtual bool isUpdated() = 0;
    virtual void printDebug() = 0;

protected:
    AbsUpdater() {}
};

class DummyUpdater : public AbsUpdater {
friend class AbsUpdaterFactory;
public:
    virtual ~DummyUpdater() {}

    virtual bool onInitialization() override
    {
        return true;
    }
    virtual bool onFinalization() override
    {
        return true;
    }

    virtual bool deploy(const string& path) override
    {
        return true;
    }
    virtual bool undeploy() override
    {
        return true;
    }
    virtual bool setReadWriteMode() override
    {
        return true;
    }
    virtual bool isUpdated() override
    {
        return false;
    }
    virtual void printDebug() override
    {

    }

private:
    DummyUpdater()
    {
        setClassName("DummyUpdater");
    }
};

class AbsUpdaterFactory {
public:
    static AbsUpdater& getInstance();

private:
    AbsUpdaterFactory() {};
    AbsUpdaterFactory(const AbsUpdaterFactory&) = delete;
    virtual ~AbsUpdaterFactory() {};
};

#endif /* UPDATER_ABSUPDATER_H_ */
