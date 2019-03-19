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

#include "Util.h"

#include <ifaddrs.h>
#include <linux/if_packet.h>
#include <sys/socket.h>
#include <unistd.h>

string Util::getMacAddress(const string& ifaceName)
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

