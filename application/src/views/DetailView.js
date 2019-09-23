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
import Divider from '@enact/moonstone/Divider';
import Scroller from '@enact/moonstone/Scroller';
import Panel from '@enact/moonstone/Panels';
import {Layout, Cell, Row, Column} from '@enact/ui/Layout';
import SoftwareModule from '../components/SoftwareModule';

class DetailView extends React.Component {
    constructor() {
        super();
    }

    render() {
        const softwareModules = this.props.softwareModules || [];
        const osModules = softwareModules.reduce((filtered, softwareModule) => {
            if (softwareModule.type === 'os') {
                filtered.push(<SoftwareModule key={filtered.length} softwareModule={softwareModule} />)
            }
            return filtered;
        }, []);
        const appModules = softwareModules.reduce((filtered, softwareModule) => {
            if (softwareModule.type === 'application') {
                filtered.push(<SoftwareModule key={filtered.length} softwareModule={softwareModule} />)
            }
            return filtered;
        }, []);

        return (
            <Panel noCloseButton>
                <Scroller>
                    <Layout orientation="vertical">
                        <Cell shrink>
                            <Divider>OS</Divider>
                        </Cell>
                        <Cell shrink>
                            {osModules}
                        </Cell>
                        <Cell shrink>
                            <Divider>App</Divider>
                        </Cell>
                        <Cell shrink>
                            {appModules}
                        </Cell>
                    </Layout>
                </Scroller>
            </Panel>
        );
    }
}

export default DetailView;