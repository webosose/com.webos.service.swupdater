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


#include <core/experimental/Composite.h>

Composite::Composite()
    : m_downloadIndex(-1)
    , m_updateIndex(-1)
{
    m_children.clear();
}

Composite::~Composite()
{
    m_children.clear();
}

void Composite::onDownloadStateChanged(enum StateType prev, enum StateType cur)
{
    if (m_download.getState() != StateType_RUNNING) {
        return;
    }

    switch (cur) {
    case StateType_NONE:
    case StateType_WAITING:
    case StateType_RUNNING:
    case StateType_PAUSED:
    case StateType_CANCELED:
        // Don't need to consider this status change. Composite already know this changes.
        return;

    case StateType_READY:
        m_children[m_downloadIndex]->startDownload();
        break;

    case StateType_COMPLETED:
        if (m_downloadIndex == m_children.size() -1) {
            m_download.complete();
            break;
        }
        m_children[++m_downloadIndex]->prepareDownload();
        return;

    case StateType_FAILED:
        m_download.fail();
        return;
    }

    if (m_download.getState() == StateType_COMPLETED && m_update.getState() == StateType_WAITING) {
        startUpdate();
    }
}

bool Composite::prepareDownload()
{
    enum TransitionType type = m_download.canPrepare();
    if (type == TransitionType_NotAllowed) {
        return false;
    } else if (type == TransitionType_Same) {
        return true;
    }
    m_downloadIndex = 0;
    if (!m_children[m_downloadIndex]->prepareDownload()) {
        m_download.fail();
        return false;
    }
    return m_download.prepare();
}

bool Composite::startDownload()
{
    enum TransitionType type = m_download.canStart();
    if (type == TransitionType_NotAllowed) {
        return false;
    } else if (type == TransitionType_Same) {
        return true;
    }
    if (!m_children[m_downloadIndex]->startDownload()) {
        m_download.fail();
        return false;
    }
    return m_download.start();
}

bool Composite::pauseDownload()
{
    enum TransitionType type = m_download.canPause();
    if (type == TransitionType_NotAllowed) {
        return false;
    } else if (type == TransitionType_Same) {
        return true;
    }
    if (!m_children[m_downloadIndex]->pauseDownload()) {
        m_download.fail();
        return false;
    }
    return m_download.pause();
}

bool Composite::resumeDownload()
{
    enum TransitionType type = m_download.canResume();
    if (type == TransitionType_NotAllowed) {
        return false;
    } else if (type == TransitionType_Same) {
        return true;
    }
    if (!m_children[m_downloadIndex]->resumeDownload()) {
        m_download.fail();
        return false;
    }
    return m_download.resume();
}

bool Composite::cancelDownload()
{
    enum TransitionType type = m_download.canCancel();
    if (type == TransitionType_NotAllowed) {
        return false;
    } else if (type == TransitionType_Same) {
        return true;
    }
    if (!m_children[m_downloadIndex]->cancelDownload()) {
        m_download.fail();
        return false;
    }
    return m_download.cancel();
}

void Composite::onUpdateStateChanged(enum StateType prev, enum StateType cur)
{
    if (m_update.getState() != StateType_RUNNING) {
        return;
    }

    switch (cur) {
    case StateType_NONE:
    case StateType_WAITING:
    case StateType_RUNNING:
    case StateType_PAUSED:
    case StateType_CANCELED:
        // Don't need to consider this status change. Composite already know this changes.
        return;

    case StateType_READY:
        m_children[m_updateIndex]->startUpdate();
        break;

    case StateType_COMPLETED:
        if (m_updateIndex == m_children.size() -1) {
            m_update.complete();
            break;
        }
        m_children[++m_updateIndex]->prepareUpdate();
        return;

    case StateType_FAILED:
        m_update.fail();
        return;
    }
}

bool Composite::prepareUpdate()
{
    enum TransitionType type = m_update.canPrepare();
    if (type == TransitionType_NotAllowed) {
        return false;
    } else if (type == TransitionType_Same) {
        return true;
    }
    m_updateIndex = 0;
    if (!m_children[m_updateIndex]->prepareUpdate()) {
        m_update.fail();
        return false;
    }
    return m_update.prepare();
}

bool Composite::startUpdate()
{
    if (m_download.getState() == StateType_COMPLETED) {
        enum TransitionType type = m_update.canStart();
        if (type == TransitionType_NotAllowed) {
            return false;
        } else if (type == TransitionType_Same) {
            return true;
        }
        m_updateIndex = 0;
        if (!m_children[m_updateIndex]->startUpdate()) {
            m_update.fail();
            return false;
        }
        return m_update.start();
    } else {
        enum TransitionType type = m_update.canWait();
        if (type == TransitionType_NotAllowed) {
            return false;
        } else if (type == TransitionType_Same) {
            return true;
        }
        return m_update.wait();
    }
}

bool Composite::pauseUpdate()
{
    enum TransitionType type = m_update.canPause();
    if (type == TransitionType_NotAllowed) {
        return false;
    } else if (type == TransitionType_Same) {
        return true;
    }
    if (!m_children[m_updateIndex]->pauseUpdate()) {
        m_update.fail();
        return false;
    }
    return m_update.pause();
}

bool Composite::resumeUpdate()
{
    enum TransitionType type = m_update.canResume();
    if (type == TransitionType_NotAllowed) {
        return false;
    } else if (type == TransitionType_Same) {
        return true;
    }
    if (!m_children[m_updateIndex]->pauseUpdate()) {
        m_update.fail();
        return false;
    }
    return m_update.resume();
}

bool Composite::cancelUpdate()
{
    enum TransitionType type = m_update.canCancel();
    if (type == TransitionType_NotAllowed) {
        return false;
    } else if (type == TransitionType_Same) {
        return true;
    }
    if (!m_children[m_updateIndex]->cancelUpdate()) {
        m_update.fail();
        return false;
    }
    return m_update.cancel();
}

void Composite::add(shared_ptr<Component> component)
{
    m_children.push_back(component);
    component->getDownload().setCallback( // @suppress("Invalid arguments")
        std::bind(&Composite::onDownloadStateChanged,
                  this,
                  std::placeholders::_1,
                  std::placeholders::_2
        )
    );
    component->getUpdate().setCallback( // @suppress("Invalid arguments")
        std::bind(&Composite::onUpdateStateChanged,
                  this,
                  std::placeholders::_1,
                  std::placeholders::_2
        )
    );
}

void Composite::remove(shared_ptr<Component> component)
{
    component->getDownload().setCallback(nullptr);
    component->getUpdate().setCallback(nullptr);
    //
}

//void DeploymentAction::addCallback()
//{
//    for (auto it = m_modules.begin(); it != m_modules.end(); ++it) {
//        it->getDownload().setCallback( // @suppress("Invalid arguments")
//            std::bind(&DeploymentAction::onDownloadStateChanged,
//                      this,
//                      std::placeholders::_1,
//                      std::placeholders::_2
//            )
//        );
//
//        it->getUpdate().setCallback( // @suppress("Invalid arguments")
//            std::bind(&DeploymentAction::onUpdateStateChanged,
//                      this,
//                      std::placeholders::_1,
//                      std::placeholders::_2
//            )
//        );
//    }
//}
//
//void DeploymentAction::removeCallback()
//{
//    for (auto it = m_modules.begin(); it != m_modules.end(); ++it) {
//        it->getDownload().setCallback(nullptr);
//        it->getUpdate().setCallback(nullptr);
//    }
//}
