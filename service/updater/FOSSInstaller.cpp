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

#include <ls2/LS2Handler.h>
#include <updater/FOSSInstaller.h>


string FOSSInstaller::toString(RECOVERY_STATUS status)
{
    switch (status) {
    case IDLE:
    case SUBPROCESS:
        return "idle";
    case START:
        return "start";
    case DOWNLOAD:
        return "download";
    case RUN:
        return "run";
    case SUCCESS:
        return "success";
    case FAILURE:
        return "failure";
    case DONE:
        return "done";
    }
    return "unknown";
}

FOSSInstaller::FOSSInstaller()
    : m_progress(0)
{
    setClassName("FOSSInstaller");
}

FOSSInstaller::~FOSSInstaller()
{
}

bool FOSSInstaller::onInitialization()
{
    return true;
}

bool FOSSInstaller::onFinalization()
{
    return true;
}

void FOSSInstaller::onRead(GIOChannel* channel)
{
    struct progress_msg msg;
    gsize len;
    string status;
    int progress;

    GIOStatus result = g_io_channel_read_chars(channel, (char*) &msg, sizeof(msg), &len, NULL);
    switch (result) {
    case G_IO_STATUS_ERROR:
        Logger::info(getClassName(), "onRead", "read error");
        break;

    case G_IO_STATUS_EOF:
        Logger::info(getClassName(), "onRead", "read eof");
        break;

    default:
        break;
    }

    // calculate progress for update
    if (msg.nsteps > 0 && msg.cur_step > 0) {
        progress = (100 * (msg.cur_step - 1) + msg.cur_percent) / msg.nsteps;
    } else {
        progress = 0;
    }

    status = toString(msg.status);
    if (msg.nsteps > 0 && msg.cur_step == 0) {
        status = "download";
        if (msg.infolen > 0) {
            JValue infoObj = JDomParser::fromString(msg.info);
            if (infoObj.hasKey("0")) {
                // RECOVERY_NO_ERROR
                progress = infoObj["0"]["percent"].asNumber<int>();
            } else if (infoObj.hasKey("1")) {
                // RECOVERY_ERROR
                // TODO
            }
        }
    }
    if (m_status == status && m_progress == progress) {
        return;
    }
    m_status = status;
    m_progress = progress;
    JValue responsePayload = Object();
    responsePayload.put("returnValue", true);
    responsePayload.put("subscribed", true);
    responsePayload.put("status", status);
    responsePayload.put("progress", progress);
    // LS2Handler::getInstance().postStatus(responsePayload);
}

bool FOSSInstaller::install(/* command line parameters */)
{
    if (m_swupdate.isRunning()) {
        m_swupdate.stop();
    }

    // 1. fork swupdate command line
    m_swupdate.setArgs("");
    m_swupdate.execute();

    // 2. connect swupdate
    m_clientSocket = make_shared<Socket>();
    m_clientSocket->setListener(this);

    return true;
}
