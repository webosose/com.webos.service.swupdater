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

import LS2Request from '@enact/webos/LS2Request';

const URI_SERVICE_NOTIFICATION = 'luna://com.webos.notification';
const URI_SERVICE_BUS = 'luna://com.webos.service.bus';

export const call = (service, method, params, completeCallback) => {
    console.log('call', service, method);
    return new LS2Request().send({
        service: service,
        method: method,
        parameters: params,
        onComplete: (res) => {
            if (completeCallback) {
                completeCallback(res);
            }
        }
    });
};

export const subscribe = (service, method, params, completeCallback) => {
    console.log('subscribe', service, method);
    return new LS2Request().send({
        service: service,
        method: method,
        subscribe: true,
        parameters: params,
        onComplete: (res) => {
            if (completeCallback) {
                completeCallback(res);
            }
        }
    });
};

export const createToast = (message) => {
    console.log('createToast');
    const params = {noaction: true, message: message};
    return call(URI_SERVICE_NOTIFICATION, 'createToast', params, (res) => {
        if (!res.returnValue) {
            console.error(res);
        }
    });
};

export const registerServerStatus = (serviceName, completeCallback) => {
    console.log('registerServerStatus');
    const params = {serviceName: serviceName};
    return subscribe(URI_SERVICE_BUS, 'signal/registerServerStatus', params, completeCallback);
};
