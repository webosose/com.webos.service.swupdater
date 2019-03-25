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

#ifndef CORE_FEEDBACK_H_
#define CORE_FEEDBACK_H_

#include <list>
#include <pbnjson.hpp>
#include <string>

using namespace pbnjson;
using namespace std;

enum FinishedType {
    FinishedType_NONE,
    FinishedType_SUCCESS,
    FinishedType_FAILURE,
};

enum ExecutionType {
    ExecutionType_CLOSED,
    ExecutionType_CANCELED,
    ExecutionType_REJECTED,
    ExecutionType_PROCEEDING,
    ExecutionType_SCHEDULED,
    ExecutionType_RESUMED,
    ExecutionType_UNKNOWN,
};

class Feedback {
public:
    Feedback(const string& actionId, ExecutionType execution, FinishedType finished);
    virtual ~Feedback();

    const string& getActionId()
    {
        return m_actionId;
    }
    void setActionId(const string& actionId)
    {
        m_actionId = actionId;
    }
    void setProgress(int of, int cnt)
    {
        m_progressOf = of;
        m_progressCnt = cnt;
    }
    void setExecution(ExecutionType execution)
    {
        m_execution = execution;
    }
    void setFinished(FinishedType finished)
    {
        m_finished = finished;
    }
    void addDetail(string detail)
    {
        m_details.push_back(detail);
    }

    virtual bool toJson(JValue& json);

private:
    static string toString(enum FinishedType type);
    static string toString(enum ExecutionType type);

    string m_actionId;
    int m_progressOf;
    int m_progressCnt;
    ExecutionType m_execution;
    FinishedType m_finished;
    list<string> m_details;
};

#endif /* CORE_FEEDBACK_H_ */
