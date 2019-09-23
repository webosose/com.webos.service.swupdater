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
import BodyText from '@enact/ui/BodyText/BodyText';
import Button from '@enact/moonstone/Button';
import CheckBoxItem from '@enact/moonstone/CheckboxItem';
import Input from '@enact/moonstone/Input';
import {Layout, Cell, Row, Column} from '@enact/ui/Layout';
import {call} from '../components/LunaAPI';

const SERVICE_SWUPDATER = 'com.webos.service.swupdater';
const URI_SERVICE_SWUPDATER = 'luna://' + SERVICE_SWUPDATER;

const PLACEHOLDER_DEVICEID = 'webOS_test';
const PLACEHOLDER_ADDRESS = 'http://10.178.84.116:8080';
const PLACEHOLDER_TOKEN = '377b83e10b9f894883e98351875151cb';
const PLACEHOLDER_USERNAME = 'admin';
const PLACEHOLDER_PASSWORD = 'admin';

class HawkBitTab extends React.Component {
    constructor (props) {
        super(props);
        this.state = {
            deviceId: PLACEHOLDER_DEVICEID,
            deviceIdToggle: true,
            address: PLACEHOLDER_ADDRESS,
            token: PLACEHOLDER_TOKEN,
            tokenToggle: false,
            username: PLACEHOLDER_USERNAME,
            password: PLACEHOLDER_PASSWORD,
        };

        this.onDeviceIdChange = this.onDeviceIdChange.bind(this);
        this.onDeviceIdToggle = this.onDeviceIdToggle.bind(this);
        this.onAddressChange = this.onAddressChange.bind(this);
        this.onTokenChange = this.onTokenChange.bind(this);
        this.onTokenToggle = this.onTokenToggle.bind(this);
        this.onUsernameChange = this.onUsernameChange.bind(this);
        this.onPasswordChange = this.onPasswordChange.bind(this);
        this.onConnectClick = this.onConnectClick.bind(this);
        this.onConnect = this.onConnect.bind(this);
    }

    onDeviceIdChange(e) {
        this.setState({deviceId: e.value});
    }

    onDeviceIdToggle(e) {
        this.setState({deviceIdToggle: e.selected});
    }

    onAddressChange(e) {
        this.setState({address: e.value});
    }

    onTokenChange(e) {
        this.setState({token: e.value});
    }

    onTokenToggle(e) {
        this.setState({tokenToggle: e.selected});
    }

    onUsernameChange(e) {
        this.setState({username: e.value});
    }

    onPasswordChange(e) {
        this.setState({password: e.value});
    }

    onConnectClick() {
        const {deviceId, deviceIdToggle, address, token, tokenToggle, username, password} = this.state;

        if (tokenToggle) {
            var xmlHttp = new window.XMLHttpRequest();
            xmlHttp.onreadystatechange = () => {
                if (xmlHttp.readyState == 4 && xmlHttp.status == 200) {
                    console.log(xmlHttp.response);
                    let response = JSON.parse(xmlHttp.responseText);
                    let token = response["authentication.gatewaytoken.key"].value;
                    call(URI_SERVICE_SWUPDATER, "connect", {
                        deviceId: deviceIdToggle ? '' : deviceId,
                        address: address,
                        token: token,
                    }, this.onConnect);
                    this.setState({token: token});
                }
            }
            xmlHttp.open("GET", address + "/rest/v1/system/configs", true);
            xmlHttp.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
            xmlHttp.setRequestHeader("Authorization", "Basic " + window.btoa(username + ":" + password));
            xmlHttp.send();
        } else {
            call(URI_SERVICE_SWUPDATER, "connect", {
                deviceId: deviceId,
                address: address,
                token: token,
            }, this.onConnect);
        }
    }

    onConnect(res) {
        console.log("onConnect", res);
    }

    render() {
        const {deviceIdToggle, tokenToggle} = this.state;
        const {serverStatus} = this.props;
        const styleRow = {margin: '0.5rem'};
        const styleKey = {marginLeft: '0.5rem', width: '12%'};
        const styleInput = {marginRight: '2%', width: '60%'};

        return (
            <div>
                <Row style={styleRow}>
                    <BodyText style={{marginLeft: '0.5rem'}}>registerServerStatus : {JSON.stringify(serverStatus, null, " ")}</BodyText>
                </Row>
                <Row style={styleRow}>
                    <BodyText style={styleKey}>Device ID : </BodyText>
                    <Input style={styleInput} onChange={this.onDeviceIdChange} disabled={deviceIdToggle} placeholder={deviceIdToggle?'':PLACEHOLDER_DEVICEID}/>
                    <CheckBoxItem defaultSelected={deviceIdToggle} onToggle={this.onDeviceIdToggle}>Auto generate</CheckBoxItem>
                </Row>
                <Row style={styleRow}>
                    <BodyText style={styleKey}>Address : </BodyText>
                    <Input style={styleInput} onChange={this.onAddressChange} placeholder={PLACEHOLDER_ADDRESS}/>
                </Row>
                <Row style={styleRow}>
                    <BodyText style={styleKey}>Token : </BodyText>
                    <Input style={styleInput} onChange={this.onTokenChange} disabled={tokenToggle} placeholder={tokenToggle?'':PLACEHOLDER_TOKEN}/>
                    <CheckBoxItem defaultSelected={tokenToggle} onToggle={this.onTokenToggle}>Login as admin</CheckBoxItem>
                </Row>
                <Row style={styleRow}>
                    <BodyText style={styleKey}>Username : </BodyText>
                    <Input style={styleInput} onChange={this.onUsernameChange} disabled={!tokenToggle} placeholder={tokenToggle?PLACEHOLDER_USERNAME:''}/>
                </Row>
                <Row style={styleRow}>
                    <BodyText style={styleKey}>Password : </BodyText>
                    <Input style={styleInput} onChange={this.onPasswordChange} disabled={!tokenToggle} placeholder={tokenToggle?PLACEHOLDER_PASSWORD:''}/>
                    <Button onClick={this.onConnectClick}>Connect</Button>
                </Row>
            </div>
        );
    }
}

export default HawkBitTab;