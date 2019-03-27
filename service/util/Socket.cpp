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

#include "Socket.h"


#include <ifaddrs.h>
#include <linux/if_packet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "util/Logger.h"

char* SOCKET_PROGRESS_PATH = "";

string Socket::getMacAddress(const string& ifaceName)
{
    struct ifaddrs* ifas;
    struct ifaddrs* ifa;

    int fd;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        return "";
    }

    if (getifaddrs(&ifas) == -1) {
        close(fd);
        return "";
    }

    char buff[18] = { 0, };
    for (ifa = ifas; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr->sa_family == AF_PACKET) {
            if (ifa->ifa_name != ifaceName) continue;
            struct sockaddr_ll* s = (struct sockaddr_ll*)ifa->ifa_addr;
            snprintf(buff, sizeof(buff), "%02x:%02x:%02x:%02x:%02x:%02x",
                     s->sll_addr[0], s->sll_addr[1], s->sll_addr[2], s->sll_addr[3], s->sll_addr[4], s->sll_addr[5]);
        }
    }

    close(fd);
    return buff;
}

gboolean Socket::onRead(GIOChannel* channel, GIOCondition condition, gpointer data)
{
    Socket* self = (Socket*) data;

    if (self == nullptr) {
        return G_SOURCE_REMOVE;
    }

    if (condition & G_IO_HUP) {
        return G_SOURCE_REMOVE;
    }

    self->m_socketListener->onRead(channel);
    // g_io_add_watch(channel, (GIOCondition)(G_IO_IN | G_IO_HUP), (GIOFunc)Socket::onRead, self);
    return G_SOURCE_CONTINUE;
}

Socket::Socket()
    : m_socket(NULL)
    , m_socketAddress(NULL)
    , m_socketListener(NULL)
{
    m_socket = g_socket_new(G_SOCKET_FAMILY_UNIX, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_DEFAULT, NULL);
    m_socketAddress = g_unix_socket_address_new(SOCKET_PROGRESS_PATH);

    if (g_socket_connect(m_socket, m_socketAddress, NULL, NULL)) {
        return;
    }

    GIOChannel* channel = g_io_channel_unix_new(g_socket_get_fd(m_socket));
    g_io_add_watch(channel, (GIOCondition)(G_IO_IN | G_IO_HUP), (GIOFunc)Socket::onRead, this);
    g_io_channel_unref(channel);
}

Socket::~Socket()
{
}

bool Socket::isConnected()
{
    return g_socket_is_connected(m_socket);
}

void Socket::setListener(SocketListener* listener)
{
    m_socketListener = listener;
}
