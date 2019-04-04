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

import PropTypes from 'prop-types';
import React from 'react';
import kind from '@enact/core/kind';
import BodyText from '@enact/moonstone/BodyText';
import Divider from '@enact/moonstone/Divider';
import ProgressBar from '@enact/moonstone/ProgressBar';
import {Layout, Cell} from '@enact/ui/Layout';

const SoftwareModuleList = kind({
    name: 'SoftwareModuleList',

    propTypes: {
        modules: PropTypes.array.isRequired
    },

    render: ({modules}) => {
        return modules.map((swmodule, moduleIndex) => {
            const artifacts = swmodule.artifacts.map((artifact, artifactIndex) => {
                let progress = artifact.size / artifact.total;
                return (
                    <Layout key={artifactIndex+1}>
                        <Cell size="10%" style={{marginLeft: '2.0rem'}}/>
                        <Cell size="55%" ><BodyText>{artifact.filename}</BodyText></Cell>
                        <Cell size="13%" style={{marginTop: '1.5rem'}}><ProgressBar backgroundProgress={progress}/></Cell>
                        <Cell size="13%" />
                    </Layout>
                );
            });
            return (
                <div key={moduleIndex}>
                    <Layout key={0}>
                    <Cell size="10%" style={{marginLeft: '2.0rem'}}><BodyText>v{swmodule.version}</BodyText></Cell>
                        <Cell size="55%" ><BodyText>{swmodule.name}</BodyText></Cell>
                        <Cell size="13%" ><BodyText>{swmodule.download}</BodyText></Cell>
                        <Cell size="13%" ><BodyText>{swmodule.update}</BodyText></Cell>
                    </Layout>
                    {artifacts}
                    <Divider style={{marginLeft: '2.0rem'}} />
                </div>
            );
        });
    }
});

export default SoftwareModuleList;