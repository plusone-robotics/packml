/*
 * Software License Agreement (Apache License)
 *
 * Copyright (c) 2018 Plus One Robotics
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once
#include "packml_sm/boost/packml_state_machine.h"
#include "packml_sm/boost/packml_transitions_continuous.h"

namespace packml_sm
{
class PackmlStateMachineContinuous : public PackmlStateMachine<PackmlTransitionsContinuous>
{
public:
  static std::shared_ptr<PackmlStateMachineContinuous> spawn()
  {
    return std::make_shared<PackmlStateMachineContinuous>();
  }
};
}
