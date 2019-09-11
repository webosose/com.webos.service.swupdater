/* @@@LICENSE
 *
 * Copyright (c) 2019 LG Electronics, Inc.
 *
 * Confidential computer software. Valid license from LG required for
 * possession, use or copying. Consistent with FAR 12.211 and 12.212,
 * Commercial Computer Software, Computer Software Documentation, and
 * Technical Data for Commercial Items are licensed to the U.S. Government
 * under vendor's standard commercial license.
 *
 * LICENSE@@@
 */

#include <iostream>
#include <glib.h>
#include <pbnjson.hpp>

#include "PolicyManager.h"
#include "Setting.h"
#include "hawkbit/HawkBitClient.h"
#include "hawkbit/HawkBitInfo.h"
#include "ls2/LS2Handler.h"
#include "updater/FOSSInstaller.h"
#include "util/Logger.h"

using namespace std;
using namespace pbnjson;

static GMainLoop *s_mainloop;

void exitDaemon(int signo)
{
    g_main_loop_quit(s_mainloop);
}

int main(int argc, char* argv[])
{
    signal(SIGTERM, exitDaemon);
    signal(SIGINT, exitDaemon);

    s_mainloop = g_main_loop_new(NULL, FALSE);
    if (argc > 1) {
        Setting::getInstance().printHelp();
        return 1;
    }

    // xxx: DON'T change initialization order.
    Setting::getInstance().initialize(s_mainloop);
    LS2Handler::getInstance().initialize(s_mainloop);
    // FOSSInstaller::getInstance().initialize(s_mainloop);
    HawkBitInfo::getInstance().initialize(s_mainloop);
    HawkBitClient::getInstance().initialize(s_mainloop);
    PolicyManager::getInstance().initialize(s_mainloop);

    Logger::verbose("Main", "Start g_mainloop");
    g_main_loop_run(s_mainloop);
    Logger::verbose("Main", "Stop g_mainloop");

    // xxx: DON'T change finalize order.
    PolicyManager::getInstance().finalize();
    HawkBitClient::getInstance().finalize();
    HawkBitInfo::getInstance().finalize();
    // FOSSInstaller::getInstance().finalize();
    LS2Handler::getInstance().finalize();
    Setting::getInstance().finalize();

    g_main_loop_unref(s_mainloop);

    return 0;
}
