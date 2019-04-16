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


#include "core/install/design/Composite.h"

Composite::Composite()
    : m_current(-1)
{
    m_children.clear();
}

Composite::~Composite()
{
    m_children.clear();
}

void Composite::onChildStatusChanged(enum StatusType prev, enum StatusType cur)
{
    if (m_status.getStatus() != StatusType_RUNNING) {
        return;
    }

    switch (cur) {
    case StatusType_NONE:
    case StatusType_READY:
    case StatusType_RUNNING:
    case StatusType_PAUSED:
    case StatusType_CANCELED:
        // Don't need to consider this status change. Composite already know this changes.
        return;

    case StatusType_COMPLETED:
        if (m_current == m_children.size() -1) {
            m_status.complete();
            break;
        }
        m_children[++m_current]->install();
        return;

    case StatusType_FAILED:
        m_status.fail();
        return;
    }
}

bool Composite::prepare()
{
    enum TransitionType type = m_status.canPrepare();
    if (type == TransitionType_NotAllowed) {
        return false;
    } else if (type == TransitionType_Same) {
        return true;
    }
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        if (!(*it)->prepare()) {
            m_status.fail();
            return false;
        }
    }

    return m_status.prepare();
}

bool Composite::install()
{
    enum TransitionType type = m_status.canInstall();
    if (type == TransitionType_NotAllowed) {
        return false;
    } else if (type == TransitionType_Same) {
        return true;
    }
    m_current = 0;
    if (!m_children[m_current]->install()) {
        m_status.fail();
        return false;
    }
    return m_status.install();
}

bool Composite::pause()
{
    enum TransitionType type = m_status.canPause();
    if (type == TransitionType_NotAllowed) {
        return false;
    } else if (type == TransitionType_Same) {
        return true;
    }
    if (!m_children[m_current]->pause()) {
        m_status.fail();
        return false;
    }
    return m_status.pause();
}

bool Composite::resume()
{
    enum TransitionType type = m_status.canResume();
    if (type == TransitionType_NotAllowed) {
        return false;
    } else if (type == TransitionType_Same) {
        return true;
    }
    if (!m_children[m_current]->resume()) {
        m_status.fail();
        return false;
    }
    return m_status.resume();
}

bool Composite::cancel()
{
    enum TransitionType type = m_status.canCancel();
    if (type == TransitionType_NotAllowed) {
        return false;
    } else if (type == TransitionType_Same) {
        return true;
    }
    if (!m_children[m_current]->cancel()) {
        m_status.fail();
        return false;
    }
    return m_status.cancel();
}

void Composite::add(shared_ptr<Component> component)
{
    m_children.push_back(component);
    component->getStatus().setCallback( // @suppress("Invalid arguments")
        std::bind(&Composite::onChildStatusChanged,
                  this,
                  std::placeholders::_1,
                  std::placeholders::_2
        )
    );
}

void Composite::remove(shared_ptr<Component> component)
{
    component->getStatus().setCallback(nullptr);
}
