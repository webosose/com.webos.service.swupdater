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


#ifndef CORE_EXPERIMENTAL_COMPOSITE_H_
#define CORE_EXPERIMENTAL_COMPOSITE_H_

#include <core/experimental/Component.h>
#include <iostream>
#include <deque>


using namespace std;

class Composite : public Component {
public:
    Composite();
    virtual ~Composite();

    virtual void onDownloadStateChanged(enum StateType prev, enum StateType cur);
    virtual bool prepareDownload() override;
    virtual bool startDownload() override;
    virtual bool pauseDownload() override;
    virtual bool resumeDownload() override;
    virtual bool cancelDownload()  override;

    virtual void onUpdateStateChanged(enum StateType prev, enum StateType cur);
    virtual bool prepareUpdate() override;
    virtual bool startUpdate() override;
    virtual bool pauseUpdate() override;
    virtual bool resumeUpdate() override;
    virtual bool cancelUpdate() override;

protected:
    void add(shared_ptr<Component> component);
    void remove(shared_ptr<Component> component);

    deque<shared_ptr<Component>> m_children;

    unsigned int m_downloadIndex;
    unsigned int m_updateIndex;

};

#endif /* CORE_EXPERIMENTAL_COMPOSITE_H_ */
