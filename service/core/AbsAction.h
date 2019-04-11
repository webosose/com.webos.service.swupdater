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

#ifndef CORE_ABSACTION_H_
#define CORE_ABSACTION_H_

#include <pbnjson.hpp>
#include <string>

#include "interface/IClassName.h"
#include "interface/ISerializable.h"

using namespace pbnjson;
using namespace std;

enum ActionType {
    ActionType_NONE,
    ActionType_INSTALL,
    ActionType_CANCEL,
};

class AbsAction : public IClassName {
public:
    AbsAction()
    {
        setClassName("AbsAction");
        m_type = ActionType_NONE;
    }

    virtual ~AbsAction() {}

    virtual const string& getId()
    {
        return m_id;
    }

    virtual void setType(ActionType type)
    {
        m_type = type;
    }

    virtual ActionType getType()
    {
        return m_type;
    }

protected:
    string m_id;

private:
    ActionType m_type;

};

#endif /* CORE_ABSACTION_H_ */
