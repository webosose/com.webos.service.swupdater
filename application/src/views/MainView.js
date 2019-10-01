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
import Divider from '@enact/moonstone/Divider/Divider';
import {Panels, Panel, Header} from '@enact/moonstone/Panels';
import {Layout, Cell, Row, Column} from '@enact/ui/Layout';
import Nav from '../components/Nav';
import {registerServerStatus} from '../components/LunaAPI';
import HawkBitTab from './HawkBitTab';
import UpdateTab from './UpdateTab';

const SERVICE_SWUPDATER = 'com.webos.service.swupdater';
const URI_SERVICE_SWUPDATER = 'luna://' + SERVICE_SWUPDATER;
const TABS = ['settings', 'update'];

class MainView extends React.Component {
    constructor (props) {
        super(props);
        this.state = {
            tab: TABS[0],
            serverStatus : {},
        };

        this.onRegisterServerStatus = this.onRegisterServerStatus.bind(this);
        this.onApplicationClose = this.onApplicationClose.bind(this);
        this.onTabChange = this.onTabChange.bind(this);
    }

    onRegisterServerStatus(res) {
        console.log('onRegisterServerStatus', res);
        this.setState({serverStatus: res});
    }

    onApplicationClose() {
        if (typeof window === 'object') {
            window.close();
        }
    }

    onTabChange(e) {
        const tab = e.target.textContent.toLowerCase();
        this.setState({tab: tab});
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
    }

    render() {
        const onTabChange = this.onTabChange;
        const selectedTab = this.state.tab;
        const serverStatus = this.state.serverStatus;

        return (
            <Panels onApplicationClose={this.onApplicationClose}>
                <Panel>
                    <Header title="S/W update demo" type="compact"/>
                    <Column>
                        <Cell shrink>
                            <Nav
                                tabs={TABS}
                                onTabChange={onTabChange}
                                selectedTab={selectedTab}
                            />
                            <Divider/>
                        </Cell>
                        <Cell>
                            {selectedTab === 'settings' && <HawkBitTab serverStatus={serverStatus}/>}
                            {selectedTab === 'update' && <UpdateTab serverStatus={serverStatus}/>}
                        </Cell>
                    </Column>
                </Panel>
            </Panels>
        );
    }
}

export default MainView;