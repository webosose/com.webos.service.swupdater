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

#include <execinfo.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "Environment.h"
//#include "Manager.h"
#include "manager/UpdateManager.h"
#include "setting/Setting.h"
#include "util/Logger.hpp"

static GMainLoop *s_mainloop;

void segfault_sig_action(int signal, siginfo_t *si, void *arg)
{
    void* addrs[20];
    char** strs;
    size_t size;

    size = backtrace(addrs, 20);
    strs = backtrace_symbols(addrs, size);

    Logger::error(MSGID_MAIN, "Segmentation fault %d", size);
    cerr << "Segmentation fault: controlmanager" << endl;
    for (unsigned i = 0; i < size; i++) {
        Logger::error(MSGID_MAIN, "%s", strs[i]);
        cerr << strs[i] << endl;
    }
    free (strs);
    exit(1);
}

void init_signal_handlers()
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(struct sigaction));
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = segfault_sig_action;
    sa.sa_flags = SA_SIGINFO;

    sigaction(SIGSEGV, &sa, NULL);
}

int main(int argc, char *argv[])
{
    init_signal_handlers();

    s_mainloop = g_main_loop_new(NULL, FALSE);

    Logger::info(MSGID_MAIN, "Initialize Manager", "");
    UpdateManager::getInstance().initialize(s_mainloop);

    try {
        Logger::info(MSGID_MAIN, "Run Manager", "");
        g_main_loop_run(s_mainloop);
        Logger::info(MSGID_MAIN, "End Manager", "");
    } catch (std::exception& e) {
        Logger::error(MSGID_MAIN, "Exception : %s", e.what());
        std::cout << "Exception: " << e.what() << std::endl;
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
