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

#ifndef CORE_EXPERIMENTAL_COMPONENT_H_
#define CORE_EXPERIMENTAL_COMPONENT_H_

#include "core/State.h"
#include "interface/ISerializable.h"

class Component : public ISerializable {
public:
    Component()
        : m_download("AbsComponent-download")
        , m_update("AbsComponent-update")
    {

    }

    virtual ~Component()
    {

    }

    virtual bool prepareDownload() = 0;
    virtual bool startDownload() = 0;
    virtual bool pauseDownload() = 0;
    virtual bool resumeDownload() = 0;
    virtual bool cancelDownload() = 0;

    virtual bool prepareUpdate() = 0;
    virtual bool startUpdate() = 0;
    virtual bool pauseUpdate() = 0;
    virtual bool resumeUpdate() = 0;
    virtual bool cancelUpdate() = 0;

    State& getDownload()
    {
        return m_download;
    }

    State& getUpdate()
    {
        return m_update;
    }

    virtual bool toJson(JValue& json) override
    {
        json.put("download", m_download.getStateStr());
        json.put("update", m_update.getStateStr());
        return true;
    }

    bool isComplete()
    {
        return (m_download.getState() == StateType_COMPLETED && m_update.getState() == StateType_COMPLETED);
    }

    bool isFailed()
    {
        return (m_download.getState() == StateType_FAILED || m_update.getState() == StateType_FAILED);
    }

protected:
    State m_download;
    State m_update;

};

#endif /* CORE_EXPERIMENTAL_COMPONENT_H_ */
