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

#include "ProgressManager.h"

#include <errno.h>
#include <fcntl.h>
#include <pbnjson.hpp>

#include "util/Logger.hpp"

using namespace pbnjson;

ProgressManager::ProgressManager()
    : m_socket(NULL)
    , m_socketAddr(NULL)
    , m_listener(NULL)
{
}

ProgressManager::~ProgressManager()
{
}

string ProgressManager::convertString(RECOVERY_STATUS status)
{
    switch (status) {
    case IDLE:
    case SUBPROCESS:
        return "idle";
    case START:
    case DOWNLOAD:
        return "download";
    case RUN:
    case SUCCESS:
        return "update";
    case FAILURE:
        return "failed";
    case DONE:
        return "done";
    }

    return "unknown";
}


gboolean ProgressManager::connect(gpointer data)
{
    Logger::info("ProgressManager", "connect", "");
    ProgressManager* self = (ProgressManager*) data;

    gboolean isConnected = g_socket_connect(self->m_socket, self->m_socketAddr, NULL, NULL);
    if (isConnected == FALSE) {
        // retry
        g_timeout_add(100, ProgressManager::connect, self);
    } else {
        Logger::info("ProgressManager", "connect %d", g_socket_get_fd(self->m_socket), "");
        GIOChannel* channel = g_io_channel_unix_new(g_socket_get_fd(self->m_socket));
        g_io_add_watch(channel, (GIOCondition)(G_IO_IN | G_IO_HUP), (GIOFunc)ProgressManager::onRead, self);
        g_io_channel_unref(channel);
    }

    return G_SOURCE_REMOVE;
}

bool ProgressManager::initialize()
{
    Logger::info("ProgressManager", "initialize: %d", (m_socket == NULL) ? -1 : g_socket_get_fd(m_socket), "");

    if (m_socket == NULL) {
        m_socket = g_socket_new(G_SOCKET_FAMILY_UNIX, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_DEFAULT, NULL);
    }
    if (m_socketAddr == NULL) {
        m_socketAddr = g_unix_socket_address_new(SOCKET_PROGRESS_PATH);
    }
    if (g_socket_is_connected(m_socket) == FALSE) {
        ProgressManager::connect(this);
    }

    return true;
}

gboolean ProgressManager::onRead(GIOChannel* channel, GIOCondition condition, gpointer data)
{
    ProgressManager* self = (ProgressManager*) data;
    struct progress_msg msg;
    gsize len;
    GIOStatus ret;
    string statusStr;
    int progress;

    Logger::info("ProgressManager", "onRead: %d", condition, "");

    if (condition & G_IO_HUP) {
        goto Reconnect;
    }


    ret = g_io_channel_read_chars(channel, (char*)&msg, sizeof(msg), &len, NULL);
    if (ret == G_IO_STATUS_ERROR) {
        Logger::info("ProgressManager", "onRead: read error", "");
        goto Reconnect;
    } else if (ret == G_IO_STATUS_EOF) {
        Logger::info("ProgressManager", "onRead: read eof", "");
    } else {
        Logger::info("ProgressManager", "onRead: read %u bytes (%u)", len, sizeof(msg), "");
    }


//    cout << " magic: " << msg.magic << ", ";
//    cout << "status: " << convertString(msg.status) << ", ";
//    cout << "dwl_percent: " << msg.dwl_percent << ", ";
//    cout << "nsteps: " << msg.nsteps << ", ";
//    cout << "cur_step: " << msg.cur_step << ", ";
//    cout << "cur_percent: " << msg.cur_percent << ", ";
//    cout << "cur_image: " << msg.cur_image << ", ";
//    cout << "hnd_name: " << msg.hnd_name << ", ";
//    cout << "source: " << msg.source << endl;
//    cout << " info: " << (msg.infolen > 0 ? msg.info : "(null)") << endl;


    if (self->m_listener) {
        // calculate progress for update
        if (msg.nsteps > 0 && msg.cur_step > 0) {
            progress = (100 * (msg.cur_step - 1) + msg.cur_percent) / msg.nsteps;
        } else {
            progress = 0;
        }

        // statusStr
        statusStr = convertString(msg.status);

        // check if download : msg.status has download status, but it does not come with download.
        if (msg.nsteps > 0 && msg.cur_step == 0) {
            statusStr = "download";

            if (msg.infolen > 0) {
                JValue infoObj = JDomParser::fromString(msg.info);
                if (infoObj.hasKey("0")) { // RECOVERY_NO_ERROR
                    progress = infoObj["0"]["percent"].asNumber<int>();
                } else if (infoObj.hasKey("1")) { // RECOVERY_ERROR
                    // TODO
                }
            }
        }

        self->m_listener->onUpdateProgress(statusStr, progress);
    }


    g_io_add_watch(channel, (GIOCondition)(G_IO_IN | G_IO_HUP), (GIOFunc)ProgressManager::onRead, self);
    return G_SOURCE_REMOVE;


Reconnect:
    g_io_channel_shutdown(channel, TRUE, NULL);
    g_socket_close(self->m_socket, NULL);
    g_object_unref(self->m_socket);
    self->m_socket = NULL;
    self->initialize();
    return G_SOURCE_REMOVE;
}

void ProgressManager::setListener(ProgressListener* listener)
{
    m_listener = listener;
}
