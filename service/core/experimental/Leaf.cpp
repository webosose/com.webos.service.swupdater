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


#include <core/experimental/Leaf.h>

Leaf::Leaf()
{
}

Leaf::~Leaf()
{
}

bool Leaf::prepareDownload()
{
    enum TransitionType type = m_download.canPrepare();
    if (type == TransitionType_NotAllowed) {
        return false;
    } else if (type == TransitionType_Same) {
        return true;
    }
    return m_download.prepare();
}

bool Leaf::startDownload()
{
    enum TransitionType type = m_download.canStart();
    if (type == TransitionType_NotAllowed) {
        return false;
    } else if (type == TransitionType_Same) {
        return true;
    }
    return m_download.start();
}

bool Leaf::pauseDownload()
{
    enum TransitionType type = m_download.canPause();
    if (type == TransitionType_NotAllowed) {
        return false;
    } else if (type == TransitionType_Same) {
        return true;
    }
    return m_download.pause();
}

bool Leaf::resumeDownload()
{
    enum TransitionType type = m_download.canResume();
    if (type == TransitionType_NotAllowed) {
        return false;
    } else if (type == TransitionType_Same) {
        return true;
    }
    return m_download.resume();
}

bool Leaf::cancelDownload()
{
    enum TransitionType type = m_download.canCancel();
    if (type == TransitionType_NotAllowed) {
        return false;
    } else if (type == TransitionType_Same) {
        return true;
    }
    return m_download.cancel();
}

bool Leaf::prepareUpdate()
{
    enum TransitionType type = m_update.canPrepare();
    if (type == TransitionType_NotAllowed) {
        return false;
    } else if (type == TransitionType_Same) {
        return true;
    }
    return m_update.prepare();
}

bool Leaf::startUpdate()
{
    enum TransitionType type = m_update.canStart();
    if (type == TransitionType_NotAllowed) {
        return false;
    } else if (type == TransitionType_Same) {
        return true;
    }
    return m_update.start();
}

bool Leaf::pauseUpdate()
{
    enum TransitionType type = m_update.canPause();
    if (type == TransitionType_NotAllowed) {
        return false;
    } else if (type == TransitionType_Same) {
        return true;
    }
    return m_update.pause();
}

bool Leaf::resumeUpdate()
{
    enum TransitionType type = m_update.canResume();
    if (type == TransitionType_NotAllowed) {
        return false;
    } else if (type == TransitionType_Same) {
        return true;
    }
    return m_update.resume();
}

bool Leaf::cancelUpdate()
{
    enum TransitionType type = m_update.canPrepare();
    if (type == TransitionType_NotAllowed) {
        return false;
    } else if (type == TransitionType_Same) {
        return true;
    }
    return m_update.prepare();
}
