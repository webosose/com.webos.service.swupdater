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
import {Layout, Cell} from '@enact/ui/Layout';
import Panel from '@enact/moonstone/Panels';

import {call} from '../components/LunaAPI';

const SERVICE_SWUPDATER = 'com.webos.service.swupdater';
const URI_SERVICE_SWUPDATER = 'luna://' + SERVICE_SWUPDATER;

class ControlView extends React.Component {
    constructor() {
        super();

        this.onResponse = this.onResponse.bind(this);
        this.onStart = this.onStart.bind(this);
        this.onPause = this.onPause.bind(this);
        this.onResume = this.onResume.bind(this);
        this.onCancel = this.onCancel.bind(this);
    }

    onResponse(res) {
        console.log('onResponse', res);
    }

    onStart() {
        call(URI_SERVICE_SWUPDATER, 'start', {}, this.onResponse);
    }

    onPause() {
        call(URI_SERVICE_SWUPDATER, 'pause', {}, this.onResponse);
    }

    onResume() {
        call(URI_SERVICE_SWUPDATER, 'resume', {}, this.onResponse);
    }

    onCancel() {
        call(URI_SERVICE_SWUPDATER, 'cancel', {}, this.onResponse);
    }

    render() {
        const {status} = this.props;
        return (
            <Panel noCloseButton>
                <Layout>
                    <Cell>
                        <Divider>Update</Divider>
                        <BodyText>Status : {status}</BodyText>
                        <Button small onClick={this.onStart}>Start</Button>
                        <Button small disabled onClick={this.onCancel}>Cancel</Button>
                        <p />
                        <Button small disabled onClick={this.onPause}>Pause</Button>
                        <Button small disabled onClick={this.onResume}>Resume</Button>
                        <p />
                    </Cell>
                </Layout>
            </Panel>
        );
    }
}

export default ControlView;