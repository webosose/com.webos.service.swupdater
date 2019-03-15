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

#include "manager/Bootloader.h"
#include "manager/FOSSInstaller.h"
#include "manager/HawkBitClient.h"
#include "manager/LS2Handler.h"


using namespace std;

static GMainLoop *s_mainloop;

void exitDaemon(int signo)
{
    g_main_loop_quit(s_mainloop);
}

int main()
{
    signal(SIGTERM, exitDaemon);
    signal(SIGINT, exitDaemon);

    s_mainloop = g_main_loop_new(NULL, FALSE);

    // xxx: DON'T change initialization order.
    Bootloader::getInstance().initialize(s_mainloop);
    LS2Handler::getInstance().initialize(s_mainloop);
    FOSSInstaller::getInstance().initialize(s_mainloop);
    HawkBitClient::getInstance().initialize(s_mainloop);

    g_main_loop_run(s_mainloop);

    // xxx: DON'T change finalize order.
    HawkBitClient::getInstance().finalize();
    FOSSInstaller::getInstance().finalize();
    LS2Handler::getInstance().finalize();
    Bootloader::getInstance().finalize();

    g_main_loop_unref(s_mainloop);

    return 0;
}
