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


#include "core/install/design/Leaf.h"

Leaf::Leaf()
{
}

Leaf::~Leaf()
{
}

bool Leaf::prepare()
{
    enum TransitionType type = m_status.canPrepare();
    if (type == TransitionType_NotAllowed) {
        return false;
    } else if (type == TransitionType_Same) {
        return true;
    }
    return m_status.prepare();
}

bool Leaf::install()
{
    enum TransitionType type = m_status.canInstall();
    if (type == TransitionType_NotAllowed) {
        return false;
    } else if (type == TransitionType_Same) {
        return true;
    }
    return m_status.install();
}

bool Leaf::pause()
{
    enum TransitionType type = m_status.canPause();
    if (type == TransitionType_NotAllowed) {
        return false;
    } else if (type == TransitionType_Same) {
        return true;
    }
    return m_status.pause();
}

bool Leaf::resume()
{
    enum TransitionType type = m_status.canResume();
    if (type == TransitionType_NotAllowed) {
        return false;
    } else if (type == TransitionType_Same) {
        return true;
    }
    return m_status.resume();
}

bool Leaf::cancel()
{
    enum TransitionType type = m_status.canCancel();
    if (type == TransitionType_NotAllowed) {
        return false;
    } else if (type == TransitionType_Same) {
        return true;
    }
    return m_status.cancel();
}
