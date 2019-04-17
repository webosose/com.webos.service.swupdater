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

#ifndef CORE_INSTALL_DESIGN_COMPONENT_H_
#define CORE_INSTALL_DESIGN_COMPONENT_H_

#include "core/Status.h"
#include "interface/ISerializable.h"

class Component : public ISerializable {
public:
    Component()
        : m_status("Component")
    {

    }

    virtual ~Component()
    {

    }

    virtual bool prepare() = 0;
    virtual bool start() = 0;
    virtual bool pause() = 0;
    virtual bool resume() = 0;
    virtual bool cancel() = 0;

    Status& getStatus()
    {
        return m_status;
    }

    virtual bool toJson(JValue& json) override
    {
        json.put("status", m_status.getStatusStr());
        return true;
    }

protected:
    Status m_status;

};

#endif /* CORE_INSTALL_DESIGN_COMPONENT_H_ */