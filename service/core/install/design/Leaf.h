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


#ifndef CORE_INSTALL_DESIGN_LEAF_H_
#define CORE_INSTALL_DESIGN_LEAF_H_

#include "core/install/design/Component.h"

class Leaf : public Component {
public:
    Leaf();
    virtual ~Leaf();

    virtual bool prepare() override;
    virtual bool start() override;
    virtual bool pause() override;
    virtual bool resume() override;
    virtual bool cancel()  override;

};

#endif /* CORE_INSTALL_DESIGN_LEAF_H_ */