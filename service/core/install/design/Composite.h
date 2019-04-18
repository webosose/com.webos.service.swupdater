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


#ifndef CORE_INSTALL_DESIGN_COMPOSITE_H_
#define CORE_INSTALL_DESIGN_COMPOSITE_H_

#include <iostream>
#include <deque>

#include "core/install/design/Component.h"

using namespace std;

class Composite : public Component {
public:
    Composite();
    virtual ~Composite();

    virtual void onChildStatusChanged(enum StatusType prev, enum StatusType cur);
    virtual bool prepare() override;
    virtual bool start() override;
    virtual bool pause() override;
    virtual bool resume() override;
    virtual bool cancel()  override;

protected:
    void enableCallback();
    void disbleCallback();

    deque<shared_ptr<Component>> m_children;

    unsigned int m_current;

};

#endif /* CORE_INSTALL_DESIGN_COMPOSITE_H_ */
