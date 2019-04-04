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

#include <string.h>
#include <Setting.h>

#include "util/Logger.h"

Setting::Setting()
    : m_onLogCurl(false)
    , m_onLogHttp(false)
    , m_verbose(false)
    , m_id("")
{
    setClassName("Setting");
}

Setting::~Setting()
{
}

void Setting::printHelp()
{
    cout << "Usage) ENV_OPTIONS /usr/sbin/swupdater"<< endl;
    cout << "Option) ID=webOS_XXXX"<< endl;
    cout << "Option) LOG_TYPE=[pmlog|console]"<< endl;
    cout << "Option) LOG_LEVEL=[verbose|debug|info|warning|error]"<< endl;
    cout << "Option) LOG_CURL=[on|off]"<< endl;
    cout << "Option) LOG_HTTP=[on|off]"<< endl;
    cout << "Example) ID=webOS_kkangeva LOG_TYPE=console LOG_LEVEL=verbose /usr/sbin/swupdater"<< endl;
}

bool Setting::onInitialization()
{
    char* env = std::getenv("ID");
    if (env) {
        m_id = env;
    }

    env = std::getenv("LOG_TYPE");
    if (env && strcmp(env, "pmlog") == 0) {
        Logger::getInstance().setType(LogType_PMLOG);
    } else if (env && strcmp(env, "console") == 0) {
        Logger::getInstance().setType(LogType_CONSOLE);
    }

    env = std::getenv("LOG_LEVEL");
    if (env && strcmp(env, "verbose") == 0) {
        Logger::getInstance().setLevel(LogLevel_VERBOSE);
        m_verbose = true;
    } else if (env && strcmp(env, "debug") == 0) {
        Logger::getInstance().setLevel(LogLevel_DEBUG);
    } else if (env && strcmp(env, "info") == 0) {
        Logger::getInstance().setLevel(LogLevel_INFO);
    } else if (env && strcmp(env, "warning") == 0) {
        Logger::getInstance().setLevel(LogLevel_WARNING);
    } else if (env && strcmp(env, "error") == 0) {
        Logger::getInstance().setLevel(LogLevel_ERROR);
    }

    env = std::getenv("LOG_CURL");
    if (env && strcmp(env, "on") == 0) {
        m_onLogCurl = true;
    }

    env = std::getenv("LOG_HTTP");
    if (env && strcmp(env, "on") == 0) {
        m_onLogHttp = true;
    }
    return true;
}

bool Setting::onFinalization()
{
    return true;
}
