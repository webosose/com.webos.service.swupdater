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
import ProgressBar from '@enact/moonstone/ProgressBar';
import {Layout, Cell} from '@enact/ui/Layout';
import Divider from '@enact/moonstone/Divider/Divider';

const SoftwareModule = kind({
    name: 'SoftwareModule',

    propTypes: {
        swmodule: PropTypes.objectOf(
            PropTypes.shape({
                type: PropTypes.string,
                name: PropTypes.string,
                version: PropTypes.string,
                download: PropTypes.string,
                update: PropTypes.string,
                artifacts: PropTypes.arrayOf(
                    PropTypes.shape({
                        size: PropTypes.number,
                        total: PropTypes.number,
                        filename: PropTypes.string,
                        download: PropTypes.string
                    })
                )
            })
        )
    },

    render: ({softwareModule}) => {
        const artifacts = softwareModule.artifacts.map((artifact, artifactIndex) => {
            let progress = artifact.size / artifact.total;
            return (
                <div key={artifactIndex}>
                    <BodyText>{artifact.filename}</BodyText>
                    <ProgressBar backgroundProgress={progress} style={{position: 'absolute', width: 'calc(100% - 1.0rem)', marginBlockStart: 0, transform: 'translateY(-1.0em)'}} />
                </div>
            );
        });

        return (
            <div>
                <Layout>
                    <Cell size="10em">
                        <BodyText>name</BodyText>
                        <BodyText>version</BodyText>
                        <BodyText>download</BodyText>
                        <BodyText>update</BodyText>
                        <BodyText>files</BodyText>
                    </Cell>
                    <Cell>
                        <BodyText>{softwareModule.name}</BodyText>
                        <BodyText>{softwareModule.version}</BodyText>
                        <BodyText>{softwareModule.download}</BodyText>
                        <BodyText>{softwareModule.update}</BodyText>
                        {artifacts}
                    </Cell>
                </Layout>
                <Divider />
            </div>
        );
    }
});

export default SoftwareModule;