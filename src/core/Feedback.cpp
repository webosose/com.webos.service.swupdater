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

#include "Feedback.h"

#include "util/Time.h"

Feedback::Feedback(const string& actionId, ExecutionType execution, FinishedType finished)
    : m_progressOf(-1)
    , m_progressCnt(-1)
{
    m_actionId = actionId;
    m_execution = execution;
    m_finished = finished;
}

Feedback::~Feedback()
{
}

bool Feedback::toJson(JValue& json)
{
    json.put("id", m_actionId);
    json.put("time", Time::getUtcTime());

    JValue result = Object();
    if (m_progressOf != -1 && m_progressCnt != -1) {
        JValue progress = Object();
        progress.put("of", m_progressOf);
        progress.put("cnt", m_progressCnt);
        result.put("progress", progress);
    }
    result.put("finished", toString(m_finished));

    JValue status = Object();
    status.put("result", result);
    status.put("execution", toString(m_execution));
    json.put("status", status);

    if (m_details.size() > 0) {
        JValue details = Array();
        for (string detail : m_details) {
            details.append(detail);
        }
        status.put("details", details);
    }
    return true;
}

string Feedback::toString(enum FinishedType& type)
{
    switch (type) {
    case FinishedType_NONE:
        return "none";
    case FinishedType_SUCCESS:
        return "success";
    case FinishedType_FAILURE:
        return "failure";
    }
    return "none";
}

string Feedback::toString(enum ExecutionType& type)
{
    switch (type) {
    case ExecutionType_CLOSED:
        return "closed";
    case ExecutionType_CANCELED:
        return "canceled";
    case ExecutionType_REJECTED:
        return "rejected";
    case ExecutionType_PROCEEDING:
        return "proceeding";
    case ExecutionType_SCHEDULED:
        return "scheduled";
    case ExecutionType_RESUMED:
        return "resumed";
    case ExecutionType_UNKNOWN:
        return "unknown";
    }
    return "unknown";
}
