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
        this.onDownloadStart = this.onDownloadStart.bind(this);
        this.onDownloadPause = this.onDownloadPause.bind(this);
        this.onDownloadResume = this.onDownloadResume.bind(this);
        this.onDownloadCancel = this.onDownloadCancel.bind(this);
        this.onUpdateStart = this.onUpdateStart.bind(this);
        this.onUpdatePause = this.onUpdatePause.bind(this);
        this.onUpdateResume = this.onUpdateResume.bind(this);
        this.onUpdateCancel = this.onUpdateCancel.bind(this);
    }

    onResponse(res) {
        console.log('onResponse', res);
    }

    onDownloadStart() {
        call(URI_SERVICE_SWUPDATER, 'download/start', {id: this.props.actionId}, this.onResponse);
    }

    onDownloadPause() {
        call(URI_SERVICE_SWUPDATER, 'download/pause', {id: this.props.actionId}, this.onResponse);
    }

    onDownloadResume() {
        call(URI_SERVICE_SWUPDATER, 'download/resume', {id: this.props.actionId}, this.onResponse);
    }

    onDownloadCancel() {
        call(URI_SERVICE_SWUPDATER, 'download/cancel', {id: this.props.actionId}, this.onResponse);
    }

    onUpdateStart() {
        call(URI_SERVICE_SWUPDATER, 'install/start', {id: this.props.actionId}, this.onResponse);
    }

    onUpdatePause() {
        call(URI_SERVICE_SWUPDATER, 'install/pause', {id: this.props.actionId}, this.onResponse);
    }

    onUpdateResume() {
        call(URI_SERVICE_SWUPDATER, 'install/resume', {id: this.props.actionId}, this.onResponse);
    }

    onUpdateCancel() {
        call(URI_SERVICE_SWUPDATER, 'install/cancel', {id: this.props.actionId}, this.onResponse);
    }

    render() {
        const {statusDownload, statusUpdate} = this.props;
        return (
            <Panel noCloseButton>
                <Layout>
                    <Cell>
                        <Divider>Download</Divider>
                        <BodyText>Status : {statusDownload}</BodyText>
                        <Button small onClick={this.onDownloadStart}>Start</Button>
                        <Button small disabled onClick={this.onDownloadCancel}>Cancel</Button>
                        <p />
                        <Button small disabled onClick={this.onDownloadPause}>Pause</Button>
                        <Button small disabled onClick={this.onDownloadResume}>Resume</Button>
                        <p />
                        <Divider />
                        <Divider>Update</Divider>
                        <BodyText>Status : {statusUpdate}</BodyText>
                        <Button small onClick={this.onUpdateStart}>Start</Button>
                        <Button small disabled onClick={this.onUpdateCancel}>Cancel</Button>
                        <p />
                        <Button small disabled onClick={this.onUpdatePause}>Pause</Button>
                        <Button small disabled onClick={this.onUpdateResume}>Resume</Button>
                        <p />
                    </Cell>
                </Layout>
            </Panel>
        );
    }
}

export default ControlView;