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
import kind from '@enact/core/kind';
import MoonstoneDecorator from '@enact/moonstone/MoonstoneDecorator';
import Panels from '@enact/moonstone/Panels';
import css from './App.module.less';
import MainPanel from '../views/MainPanel';

const App = kind({
    name: 'App',

    styles: {
        css,
        className: 'app'
    },

    handlers: {
        onApplicationClose: () => {
            if (typeof window === 'object') {
                window.close();
            }
        }
    },

    render: (props) => (
        <div {...props}>
            <Panels onApplicationClose={props.onApplicationClose}>
                <MainPanel {...props}/>
            </Panels>
        </div>
    )
});

export default MoonstoneDecorator(App);
