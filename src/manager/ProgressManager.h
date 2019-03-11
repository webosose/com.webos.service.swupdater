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

#ifndef _WRAPPER_PROGRESSMANAGER_H_
#define _WRAPPER_PROGRESSMANAGER_H_

#include <gio/gio.h>
#include <gio/gunixsocketaddress.h>
#include <glib.h>
#include <glib-object.h>
#include <iostream>

extern "C" {
#include <progress_ipc.h>
}

#include "listener/ProgressListener.h"

using namespace std;

class ProgressManager
{
public:
    virtual ~ProgressManager();

    static ProgressManager& getInstance()
    {
        static ProgressManager _instance;
        return _instance;
    }

    static string convertString(RECOVERY_STATUS status);

    static gboolean connect(gpointer data);
    static gboolean onRead(GIOChannel* channel, GIOCondition condition, gpointer data);

    bool initialize();

    void setListener(ProgressListener* listener);


private:
    GSocket* m_socket;
    GSocketAddress* m_socketAddr;
    ProgressListener* m_listener;
    string m_prevStatus;

    ProgressManager();
};

#endif /* _WRAPPER_PROGRESSMANAGER_H_ */
