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
import {Layout, Cell} from '@enact/ui/Layout';
import {Panels, Panel, Header} from '@enact/moonstone/Panels';

import {createToast, registerServerStatus, call, subscribe} from '../components/LunaAPI';
import ControlView from './ControlView';
import DetailView from './DetailView';

const SERVICE_SWUPDATER = 'com.webos.service.swupdater';
const URI_SERVICE_SWUPDATER = 'luna://' + SERVICE_SWUPDATER;
const ID_NONE = "";
const STATUS_NONE = "none";
const STATUS_COMPLETED = "completed";

class MainView extends React.Component {
    constructor() {
        super();
        this.state = {
            id: ID_NONE,
            statusDownload: STATUS_NONE,
            statusUpdate: STATUS_NONE,
            softwareModules: [],
        };

        this.onGetStatus = this.onGetStatus.bind(this);
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
        console.log('componentDidMount')
        this.registerServerStatus = registerServerStatus(SERVICE_SWUPDATER, this.onRegisterServerStatus);
    }

    componentWillUnmount() {
        console.log('componentWillUnmount')
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
        const {statusDownload, statusUpdate, softwareModules} = this.state;

        return (
            <Panels>
                <Panel>
                    <Header title="S/W update demo" />
                    <Layout style={{height: '100%'}}>
                        <Cell size="25%">
                            <ControlView actionId={this.state.id} statusDownload={statusDownload} statusUpdate={statusUpdate}/>
                        </Cell>
                        <Cell>
                            <DetailView softwareModules={softwareModules}/>
                        </Cell>
                    </Layout>
                </Panel>
            </Panels>
        );
    }
}

export default MainView;