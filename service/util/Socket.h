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

#include <iostream>
#include <glib.h>
#include <gio/gunixsocketaddress.h>

#include "interface/IListener.h"

using namespace std;

class SocketListener {
public:
    SocketListener() {};
    virtual ~SocketListener() {};

    virtual void onRead(GIOChannel* channel) = 0;
};

class Socket : public IListener<SocketListener> {
public:
    static string getMacAddress(const string& ifaceName);

    Socket();
    virtual ~Socket();

    bool isConnected();

private:
    static gboolean onRead(GIOChannel* channel, GIOCondition condition, gpointer data);

    GSocket* m_socket;
    GSocketAddress* m_socketAddress;

};

#endif /* _WRAPPER_PROGRESSMANAGER_H_ */
