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

#ifndef CORE_INSTALL_IMPL_DEPLOYMENTACTIONCOMPOSITE_H_
#define CORE_INSTALL_IMPL_DEPLOYMENTACTIONCOMPOSITE_H_

#include <iostream>
#include <deque>
#include <map>

#include "core/AbsAction.h"
#include "core/install/design/Composite.h"
#include "core/install/impl/SoftwareModuleComposite.h"

using namespace std;

class DeploymentActionComposite : public AbsAction,
                                  public Composite {
public:
    DeploymentActionComposite();
    virtual ~DeploymentActionComposite();

    void onStatusChanged(enum StatusType prev, enum StatusType cur);

    // ISerializable
    virtual bool fromJson(const JValue& json) override;
    virtual bool toJson(JValue& json) override;

    const bool isForceDownload()
    {
        return m_isForceDownload;
    }

    const bool isForceUpdate()
    {
        return m_isForceUpdate;
    }

    bool isOnlyOSModuleCompleted();
    bool toProceedingJson(JValue& json);
    bool restoreActionHistory(const JValue& json, bool isRebootDetected);

private:
    bool m_isForceDownload;
    bool m_isForceUpdate;

};

#endif /* CORE_INSTALL_IMPL_DEPLOYMENTACTIONCOMPOSITE_H_ */
