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

#include "Time.h"

#include <string.h>
#include <time.h>

long Time::getSystemTime()
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
        return 0;
    }
    return ts.tv_sec;
}

string Time::getUtcTime()
{
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        return "";
    }

    char buff[32];
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    gmtime_r(&ts.tv_sec, &tm);
    strftime(buff, sizeof(buff), "%Y%m%dT%H%M%S", &tm);

    return buff;
}

int Time::toSeconds(string& str)
{
    int hours, minutes, seconds, total = 0;
    sscanf(str.c_str(), "%d:%d:%d", &hours, &minutes, &seconds);
    total = hours * 3600 + minutes * 60 + seconds;
    return total;
}
