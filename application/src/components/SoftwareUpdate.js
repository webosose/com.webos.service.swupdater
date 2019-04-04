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

import React from 'react';
import BodyText from '@enact/moonstone/BodyText';
import Button from '@enact/moonstone/Button';
import Divider from '@enact/moonstone/Divider';
import Scroller from '@enact/moonstone/Scroller';
import {Layout, Cell} from '@enact/ui/Layout';

import {createToast, registerServerStatus, call, subscribe} from './LunaAPI';
import SoftwareModuleList from './SoftwareModuleList';

const SERVICE_SWUPDATER = 'com.webos.service.swupdater';
const URI_SERVICE_SWUPDATER = 'luna://' + SERVICE_SWUPDATER;
const ID_NONE = "";
const STATUS_NONE = "none";
const STATUS_COMPLETED = "completed";

class SoftwareUpdate extends React.Component {
    constructor (props) {
        super(props);
        this.state = {
            id: ID_NONE,
            statusDownload: STATUS_NONE,
            statusUpdate: STATUS_NONE,
            softwareModules: [],
        };

        this.onGetStatus = this.onGetStatus.bind(this);
        this.onResponse = this.onResponse.bind(this);
        this.onDownloadStart = this.onDownloadStart.bind(this);
        this.onDownloadPause = this.onDownloadPause.bind(this);
        this.onDownloadResume = this.onDownloadResume.bind(this);
        this.onDownloadCancel = this.onDownloadCancel.bind(this);
        this.onUpdateStart = this.onUpdateStart.bind(this);
        this.onUpdatePause = this.onUpdatePause.bind(this);
        this.onUpdateResume = this.onUpdateResume.bind(this);
        this.onUpdateCancel = this.onUpdateCancel.bind(this);
        this.onRegisterServerStatus = this.onRegisterServerStatus.bind(this);
    }

    onGetStatus(res) {
        const id = res.id || ID_NONE;
        const statusDownload = res.download || STATUS_NONE;
        const statusUpdate = res.update || STATUS_NONE;
        const softwareModules = res.softwareModules || [];

        if (id !== ID_NONE && this.state.id === ID_NONE) {
            createToast(`Update available<br> ${softwareModules[0].name} (v${softwareModules[0].version})`);
        }
        if (statusDownload === STATUS_COMPLETED && statusUpdate === STATUS_COMPLETED &&
                (this.state.statusDownload !== STATUS_COMPLETED || this.state.statusUpdate !== STATUS_COMPLETED)) {
            createToast(`Update completed<br> ${softwareModules[0].name} (v${softwareModules[0].version})`);
        }

        this.setState({
            id: id,
            statusDownload: statusDownload,
            statusUpdate: statusUpdate,
            softwareModules: softwareModules,
        });
    }

    onResponse(res) {
        console.log('onResponse', res);
    }

    onDownloadStart() {
        call(URI_SERVICE_SWUPDATER, 'download/start', {id: this.state.id}, this.onResponse);
    }

    onDownloadPause() {
        call(URI_SERVICE_SWUPDATER, 'download/pause', {id: this.state.id}, this.onResponse);
    }

    onDownloadResume() {
        call(URI_SERVICE_SWUPDATER, 'download/resume', {id: this.state.id}, this.onResponse);
    }

    onDownloadCancel() {
        call(URI_SERVICE_SWUPDATER, 'download/cancel', {id: this.state.id}, this.onResponse);
    }

    onUpdateStart() {
        call(URI_SERVICE_SWUPDATER, 'install/start', {id: this.state.id}, this.onResponse);
    }

    onUpdatePause() {
        call(URI_SERVICE_SWUPDATER, 'install/pause', {id: this.state.id}, this.onResponse);
    }

    onUpdateResume() {
        call(URI_SERVICE_SWUPDATER, 'install/resume', {id: this.state.id}, this.onResponse);
    }

    onUpdateCancel() {
        call(URI_SERVICE_SWUPDATER, 'install/cancel', {id: this.state.id}, this.onResponse);
    }

    onRegisterServerStatus(res) {
        console.log('onRegisterServerStatus', res);
        if (res.connected) {
            this.getStatus = subscribe(URI_SERVICE_SWUPDATER, 'getStatus', {}, this.onGetStatus);
        } else {
            if (this.getStatus) {
                this.getStatus.cancel();
                this.getStatus = null;
            }
        }
    }

    componentDidMount() {
        this.registerServerStatusSubsciption = registerServerStatus(SERVICE_SWUPDATER, this.onRegisterServerStatus);
    }

    componentWillUnmount() {
        if (this.registerServerStatus) {
            this.registerServerStatus.cancel();
            this.registerServerStatus = null;
        }
        if (this.getStatus) {
            this.getStatus.cancel();
            this.getStatus = null;
        }
    }

    render() {
        const osModules = this.state.softwareModules.filter((softwareModule) => {
            return (softwareModule.type === 'os');
        });
        const appModules = this.state.softwareModules.filter((softwareModule) => {
            return (softwareModule.type === 'application');
        });

        return (
            <Scroller>
                <div>
                    <Layout style={{marginTop: '1.0rem'}}>
                        <Cell>
                            <Divider>Download</Divider>
                            <BodyText>Status : {this.state.statusDownload}</BodyText>
                            <Button onClick={this.onDownloadStart}>Start</Button>
                            <Button disabled onClick={this.onDownloadPause}>Pause</Button>
                            <Button disabled onClick={this.onDownloadResume}>Resume</Button>
                            <Button disabled onClick={this.onDownloadCancel}>Cancel</Button>
                        </Cell>
                        <Cell>
                            <Divider>Update</Divider>
                            <BodyText>Status : {this.state.statusUpdate}</BodyText>
                            <Button onClick={this.onUpdateStart}>Start</Button>
                            <Button disabled onClick={this.onUpdatePause}>Pause</Button>
                            <Button disabled onClick={this.onUpdateResume}>Resume</Button>
                            <Button disabled onClick={this.onUpdateCancel}>Cancel</Button>
                        </Cell>
                    </Layout>
                    <br />
                    <Divider>OS</Divider>
                    <SoftwareModuleList modules={osModules} />
                    <br />
                    <Divider>App</Divider>
                    <SoftwareModuleList modules={appModules} />
                    <br />
                </div>
            </Scroller>
        )
    }
}

export default SoftwareUpdate;
