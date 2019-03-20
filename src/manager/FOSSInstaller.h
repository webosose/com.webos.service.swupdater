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

#ifndef MANAGER_FOSSINSTALLER_H_
#define MANAGER_FOSSINSTALLER_H_

#include <iostream>
#include <glib.h>

#include <pbnjson.hpp>
#include <luna-service2/lunaservice.hpp>

#include "interface/IManageable.h"
#include "util/Process.h"
#include "util/Socket.h"

#if defined(LIBSWUPDATE)
    typedef enum {
           IDLE,
           START,
           RUN,
           SUCCESS,
           FAILURE,
           DOWNLOAD,
           DONE,
           SUBPROCESS,
    } RECOVERY_STATUS;

    typedef enum {
           SOURCE_UNKNOWN,
           SOURCE_WEBSERVER,
           SOURCE_SURICATTA,
           SOURCE_DOWNLOADER,
           SOURCE_LOCAL
    } sourcetype;

    /*
    * Message sent via progress socket.
    * Data is sent in LE if required.
    */
    struct progress_msg {
           unsigned int    magic;          /* Magic Number */
           RECOVERY_STATUS status;         /* Update Status (Running, Failure) */
           unsigned int    dwl_percent;    /* % downloaded data */
           unsigned int    nsteps;         /* No. total of steps */
           unsigned int    cur_step;       /* Current step index */
           unsigned int    cur_percent;    /* % in current step */
           char            cur_image[256]; /* Name of image to be installed */
           char            hnd_name[64];   /* Name of running hanler */
           sourcetype      source;         /* Interface that triggered the update */
           unsigned int    infolen;        /* Len of data valid in info */
           char            info[2048];     /* additional information about install */
    };
#else
    #include <progress_ipc.h>
#endif

using namespace std;
using namespace pbnjson;

class FOSSInstaller : public IManageable<FOSSInstaller>, public SocketListener {
friend IManageable<FOSSInstaller>;
public:
    static string toString(RECOVERY_STATUS status);

    virtual ~FOSSInstaller();

    // IManageable
    virtual bool onInitialization() override;
    virtual bool onFinalization() override;

    // SocketListener
    void onRead(GIOChannel* channel);

    virtual bool install();

private:
    FOSSInstaller();

    Process m_swupdate;
    shared_ptr<Socket> m_clientSocket;
    string m_status;
    int m_progress;
};

#endif /* MANAGER_FOSSINSTALLER_H_ */
