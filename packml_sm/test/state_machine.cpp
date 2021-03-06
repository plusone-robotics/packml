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

#include <gtest/gtest.h>
#include "state_machine_visited_states_queue.h"
#include "packml_sm/common.h"
#include "packml_sm/abstract_state_machine.h"
#include "packml_sm/boost/packml_state_machine_single_cycle.h"
#include "packml_sm/boost/packml_state_machine_continuous.h"
#include "ros/duration.h"
#include "ros/console.h"
#include "ros/rate.h"
#include "packml_sm/packml_stats_snapshot.h"
#include "packml_sm/packml_stats_itemized.h"

namespace packml_sm_test
{
using namespace packml_sm;

/**
 * @brief waitForState - returns true if the current state of the state machine (sm) matches the queried state
 * @param state - state enumeration to wait for
 * @param sm - top level state machine
 * @return true if state changes to queried state before internal timeout
 */
bool waitForState(StatesEnum state, StateMachineVisitedStatesQueue& queue)
{
  auto i = 0;
  do
  {
    auto nextState = queue.nextState();

    if (nextState == static_cast<int>(state))
    {
      return true;
    }
    else if (nextState != -1)
    {
      return false;
    }

    ROS_WARN_STREAM("Waiting for state to change to " << state);
    ros::Duration(0.05).sleep();
    i++;
  } while (i < 25);

  return false;
}

int success()
{
  ROS_INFO_STREAM("Beginning success() method");
  ros::Duration(1.0).sleep();
  ROS_INFO_STREAM("Execute method complete");
  return 0;
}

int fail()
{
  ROS_INFO_STREAM("Beginning fail method");
  ros::Duration(1.0).sleep();
  ROS_INFO_STREAM("Execute method complete");
  return -1;
}

TEST(Packml_SM, set_execute)
{
  std::shared_ptr<PackmlStateMachineSingleCycle> sm = PackmlStateMachineSingleCycle::spawn();
  StateMachineVisitedStatesQueue queue(sm);
  sm->setExecute(std::bind(success));
  sm->activate();
  ros::Duration(1.0).sleep();  // give time to start
  ASSERT_TRUE(waitForState(StatesEnum::ABORTED, queue));
  ASSERT_TRUE(sm->clear());
  ASSERT_TRUE(waitForState(StatesEnum::CLEARING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::STOPPED, queue));
  ASSERT_TRUE(sm->reset());
  ASSERT_TRUE(waitForState(StatesEnum::RESETTING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::IDLE, queue));
  ASSERT_TRUE(sm->start());
  ASSERT_TRUE(waitForState(StatesEnum::STARTING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::EXECUTE, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::COMPLETING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::COMPLETE, queue));
  ASSERT_TRUE(sm->reset());
  ASSERT_TRUE(waitForState(StatesEnum::RESETTING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::IDLE, queue));
}

TEST(Packml_SC, state_diagram)
{
  ROS_INFO_STREAM("SINGLE CYCLE::State diagram");
  std::shared_ptr<PackmlStateMachineSingleCycle> sm = PackmlStateMachineSingleCycle::spawn();
  StateMachineVisitedStatesQueue queue(sm);

  EXPECT_FALSE(sm->isActive());
  sm->setExecute(std::bind(success));
  sm->activate();
  ros::Duration(1.0).sleep();  // give time to start
  EXPECT_TRUE(sm->isActive());

  ASSERT_TRUE(waitForState(StatesEnum::ABORTED, queue));
  ASSERT_TRUE(sm->isActive());

  ASSERT_TRUE(sm->clear());
  ASSERT_TRUE(waitForState(StatesEnum::CLEARING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::STOPPED, queue));

  ASSERT_TRUE(sm->reset());
  ASSERT_TRUE(waitForState(StatesEnum::RESETTING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::IDLE, queue));

  ASSERT_TRUE(sm->start());
  ASSERT_TRUE(waitForState(StatesEnum::STARTING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::EXECUTE, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::COMPLETING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::COMPLETE, queue));

  ASSERT_TRUE(sm->reset());
  ASSERT_TRUE(waitForState(StatesEnum::RESETTING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::IDLE, queue));

  ASSERT_TRUE(sm->start());
  ASSERT_TRUE(waitForState(StatesEnum::STARTING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::EXECUTE, queue));
  ASSERT_TRUE(sm->hold());
  ASSERT_TRUE(waitForState(StatesEnum::HOLDING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::HELD, queue));
  ASSERT_TRUE(sm->unhold());
  ASSERT_TRUE(waitForState(StatesEnum::UNHOLDING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::EXECUTE, queue));

  ASSERT_TRUE(sm->suspend());
  ASSERT_TRUE(waitForState(StatesEnum::SUSPENDING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::SUSPENDED, queue));
  ASSERT_TRUE(sm->unsuspend());
  ASSERT_TRUE(waitForState(StatesEnum::UNSUSPENDING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::EXECUTE, queue));

  ASSERT_TRUE(sm->stop());
  ASSERT_TRUE(waitForState(StatesEnum::STOPPING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::STOPPED, queue));

  ASSERT_TRUE(sm->abort());
  ASSERT_TRUE(waitForState(StatesEnum::ABORTING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::ABORTED, queue));

  sm->deactivate();
  ros::Duration(1).sleep();
  EXPECT_FALSE(sm->isActive());
  ROS_INFO_STREAM("State diagram test complete");
}

TEST(Packml_CC, state_diagram)
{
  ROS_INFO_STREAM("CONTINUOUS CYCLE::State diagram");
  std::shared_ptr<PackmlStateMachineContinuous> sm = PackmlStateMachineContinuous::spawn();
  StateMachineVisitedStatesQueue queue(sm);
  EXPECT_FALSE(sm->isActive());
  sm->setExecute(std::bind(success));
  sm->activate();
  ros::Duration(1.0).sleep();  // give time to start
  EXPECT_TRUE(sm->isActive());

  ASSERT_TRUE(waitForState(StatesEnum::ABORTED, queue));
  ASSERT_TRUE(sm->isActive());

  ASSERT_TRUE(sm->clear());
  ASSERT_TRUE(waitForState(StatesEnum::CLEARING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::STOPPED, queue));

  ASSERT_TRUE(sm->reset());
  ASSERT_TRUE(waitForState(StatesEnum::RESETTING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::IDLE, queue));

  ASSERT_TRUE(sm->start());
  ASSERT_TRUE(waitForState(StatesEnum::STARTING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::EXECUTE, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_FALSE(waitForState(StatesEnum::COMPLETING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_FALSE(waitForState(StatesEnum::COMPLETE, queue));

  ASSERT_TRUE(sm->hold());
  ASSERT_TRUE(waitForState(StatesEnum::HOLDING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::HELD, queue));

  ASSERT_TRUE(sm->unhold());
  ASSERT_TRUE(waitForState(StatesEnum::UNHOLDING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::EXECUTE, queue));

  ASSERT_TRUE(sm->suspend());
  ASSERT_TRUE(waitForState(StatesEnum::SUSPENDING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::SUSPENDED, queue));

  ASSERT_TRUE(sm->unsuspend());
  ASSERT_TRUE(waitForState(StatesEnum::UNSUSPENDING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::EXECUTE, queue));

  ASSERT_TRUE(sm->stop());
  ASSERT_TRUE(waitForState(StatesEnum::STOPPING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::STOPPED, queue));

  ASSERT_TRUE(sm->abort());
  ASSERT_TRUE(waitForState(StatesEnum::ABORTING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::ABORTED, queue));

  sm->deactivate();
  ros::Duration(1).sleep();
  EXPECT_FALSE(sm->isActive());
  ROS_INFO_STREAM("State diagram test complete");
}

TEST(Packml_CC, load_stats)
{
  ROS_INFO_STREAM("CONTINUOUS CYCLE::Load stats");
  std::shared_ptr<AbstractStateMachine> sm = PackmlStateMachineContinuous::spawn();

  packml_sm::PackmlStatsSnapshot snapshot_read;
  sm->getCurrentStatSnapshot(snapshot_read);
  ASSERT_EQ(snapshot_read.idle_duration, 0);
  ASSERT_EQ(snapshot_read.exe_duration, 0);
  ASSERT_EQ(snapshot_read.held_duration, 0);
  ASSERT_EQ(snapshot_read.susp_duration, 0);
  ASSERT_EQ(snapshot_read.cmplt_duration, 0);
  ASSERT_EQ(snapshot_read.stop_duration, 0);
  ASSERT_EQ(snapshot_read.abort_duration, 0);
  ASSERT_EQ(snapshot_read.success_count, 0);
  ASSERT_EQ(snapshot_read.fail_count, 0);
  ASSERT_EQ(snapshot_read.itemized_error_map.size(), 0);
  ASSERT_EQ(snapshot_read.itemized_quality_map.size(), 0);

  packml_sm::PackmlStatsSnapshot snapshot_load;
  snapshot_load.idle_duration = 1;
  snapshot_load.exe_duration = 2;
  snapshot_load.held_duration = 3;
  snapshot_load.susp_duration = 4;
  snapshot_load.cmplt_duration = 5;
  snapshot_load.stop_duration = 6;
  snapshot_load.abort_duration = 7;
  snapshot_load.success_count = 8;
  snapshot_load.fail_count = 9;

  std::map<int16_t, packml_sm::PackmlStatsItemized> itemized_error_map;
  packml_sm::PackmlStatsItemized error_item;
  error_item.id = 123;
  error_item.count = 456;
  error_item.duration = 789;
  itemized_error_map.insert(std::pair<int16_t, packml_sm::PackmlStatsItemized>(error_item.id, error_item));
  snapshot_load.itemized_error_map = itemized_error_map;

  std::map<int16_t, packml_sm::PackmlStatsItemized> itemized_quality_map;
  packml_sm::PackmlStatsItemized quality_item;
  quality_item.id = 111;
  quality_item.count = 222;
  quality_item.duration = 333;
  itemized_quality_map.insert(std::pair<int16_t, packml_sm::PackmlStatsItemized>(quality_item.id, quality_item));
  snapshot_load.itemized_quality_map = itemized_quality_map;

  sm->loadStats(snapshot_load);

  sm->getCurrentStatSnapshot(snapshot_read);
  ASSERT_EQ(snapshot_read.idle_duration, 1);
  ASSERT_EQ(snapshot_read.exe_duration, 2);
  ASSERT_EQ(snapshot_read.held_duration, 3);
  ASSERT_EQ(snapshot_read.susp_duration, 4);
  ASSERT_EQ(snapshot_read.cmplt_duration, 5);
  ASSERT_EQ(snapshot_read.stop_duration, 6);
  ASSERT_EQ(snapshot_read.abort_duration, 7);
  ASSERT_EQ(snapshot_read.success_count, 8);
  ASSERT_EQ(snapshot_read.fail_count, 9);
  ASSERT_EQ(snapshot_read.itemized_error_map.at(123).id, 123);
  ASSERT_EQ(snapshot_read.itemized_error_map.at(123).count, 456);
  ASSERT_EQ(snapshot_read.itemized_error_map.at(123).duration, 789);
  ASSERT_EQ(snapshot_read.itemized_quality_map.at(111).id, 111);
  ASSERT_EQ(snapshot_read.itemized_quality_map.at(111).count, 222);
  ASSERT_EQ(snapshot_read.itemized_quality_map.at(111).duration, 333);

  ROS_INFO_STREAM("load stats test complete");
}

TEST(Packml_CC, getCurrentStatTransaction_empty)
{
  ROS_INFO_STREAM("CONTINUOUS CYCLE::stats transaction empty");
  std::shared_ptr<AbstractStateMachine> sm = PackmlStateMachineContinuous::spawn();

  packml_sm::PackmlStatsSnapshot snapshot_out;
  sm->getCurrentIncrementalStatSnapshot(snapshot_out);

  ASSERT_LT(snapshot_out.duration, 1);
  ASSERT_EQ(snapshot_out.idle_duration, 0);
  ASSERT_EQ(snapshot_out.exe_duration, 0);
  ASSERT_EQ(snapshot_out.held_duration, 0);
  ASSERT_EQ(snapshot_out.susp_duration, 0);
  ASSERT_EQ(snapshot_out.cmplt_duration, 0);
  ASSERT_EQ(snapshot_out.stop_duration, 0);
  ASSERT_EQ(snapshot_out.abort_duration, 0);

  ASSERT_EQ(snapshot_out.success_count, 0);
  ASSERT_EQ(snapshot_out.fail_count, 0);
  ASSERT_EQ(snapshot_out.cycle_count, 0);
  ASSERT_EQ(snapshot_out.throughput, 0);

  ASSERT_EQ(snapshot_out.availability, 1);
  ASSERT_EQ(snapshot_out.performance, 0);
  ASSERT_EQ(snapshot_out.quality, 0);
  ASSERT_EQ(snapshot_out.overall_equipment_effectiveness, 0);

  ASSERT_EQ(snapshot_out.itemized_error_map.size(), 0);
  ASSERT_EQ(snapshot_out.itemized_quality_map.size(), 0);

  ROS_INFO_STREAM("stats transaction empty complete");
}

TEST(Packml_CC, getCurrentStatTransaction_durations)
{
  ROS_INFO_STREAM("CONTINUOUS CYCLE::stats transaction durations");
  std::shared_ptr<PackmlStateMachineContinuous> sm = PackmlStateMachineContinuous::spawn();
  StateMachineVisitedStatesQueue queue(sm);
  packml_sm::PackmlStatsSnapshot snapshot_out;

  sm->setExecute(std::bind(success));
  sm->activate();
  while (!sm->isActive()) { }

  ASSERT_TRUE(waitForState(StatesEnum::ABORTED, queue));
  ros::Duration(2.0).sleep();

  ASSERT_TRUE(sm->clear());
  ASSERT_TRUE(waitForState(StatesEnum::CLEARING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::STOPPED, queue));
  ros::Duration(2.0).sleep();

  ASSERT_TRUE(sm->reset());
  ASSERT_TRUE(waitForState(StatesEnum::RESETTING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::IDLE, queue));
  ros::Duration(2.0).sleep();

  ASSERT_TRUE(sm->start());
  ASSERT_TRUE(waitForState(StatesEnum::STARTING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::EXECUTE, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_FALSE(waitForState(StatesEnum::COMPLETING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_FALSE(waitForState(StatesEnum::COMPLETE, queue));

  ASSERT_TRUE(sm->hold());
  ASSERT_TRUE(waitForState(StatesEnum::HOLDING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::HELD, queue));

  ASSERT_TRUE(sm->unhold());
  ASSERT_TRUE(waitForState(StatesEnum::UNHOLDING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::EXECUTE, queue));

  ASSERT_TRUE(sm->suspend());
  ASSERT_TRUE(waitForState(StatesEnum::SUSPENDING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::SUSPENDED, queue));
  ros::Duration(2.0).sleep();

  ASSERT_TRUE(sm->unsuspend());
  ASSERT_TRUE(waitForState(StatesEnum::UNSUSPENDING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::EXECUTE, queue));

  ASSERT_TRUE(sm->stop());
  ASSERT_TRUE(waitForState(StatesEnum::STOPPING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::STOPPED, queue));
  ros::Duration(2.0).sleep();

  ASSERT_TRUE(sm->abort());
  ASSERT_TRUE(waitForState(StatesEnum::ABORTING, queue));
  sm->triggerEvent(state_complete_event());
  ASSERT_TRUE(waitForState(StatesEnum::ABORTED, queue));
  ros::Duration(2.0).sleep();

  sm->getCurrentIncrementalStatSnapshot(snapshot_out);
  ASSERT_NEAR(snapshot_out.duration, 16, 1);
  ASSERT_NEAR(snapshot_out.abort_duration, 4, 1);
  ASSERT_NEAR(snapshot_out.stop_duration, 4, 1);
  ASSERT_NEAR(snapshot_out.idle_duration, 2, 1);
  ASSERT_NEAR(snapshot_out.exe_duration, 4, 1);
  ASSERT_NEAR(snapshot_out.cmplt_duration, 0, 1);
  ASSERT_NEAR(snapshot_out.susp_duration, 2, 1);
  ASSERT_NEAR(snapshot_out.held_duration, 0, 1);

  // Ensure durations reset after transaction
  sm->getCurrentIncrementalStatSnapshot(snapshot_out);
  ASSERT_NEAR(snapshot_out.duration, 0, 1);
  ASSERT_NEAR(snapshot_out.abort_duration, 0, 1);
  ASSERT_NEAR(snapshot_out.stop_duration, 0, 1);
  ASSERT_NEAR(snapshot_out.idle_duration, 0, 1);
  ASSERT_NEAR(snapshot_out.exe_duration, 0, 1);
  ASSERT_NEAR(snapshot_out.cmplt_duration, 0, 1);
  ASSERT_NEAR(snapshot_out.susp_duration, 0, 1);
  ASSERT_NEAR(snapshot_out.held_duration, 0, 1);

  // Next transaction
  ros::Duration(2.0).sleep();
  sm->getCurrentIncrementalStatSnapshot(snapshot_out);
  ASSERT_NEAR(snapshot_out.duration, 2, 1);
  ASSERT_NEAR(snapshot_out.abort_duration, 2, 1);

  ROS_INFO_STREAM("stats transaction durations complete");
}

TEST(Packml_CC, getCurrentStatTransaction_counts)
{
  ROS_INFO_STREAM("CONTINUOUS CYCLE::stats transaction counts");
  std::shared_ptr<AbstractStateMachine> sm = PackmlStateMachineContinuous::spawn();
  packml_sm::PackmlStatsSnapshot snapshot_out;

  auto num_successes = 90;
  auto num_failures = 10;
  for (auto i = 0; i < num_successes; ++i)
  {
    sm->incrementSuccessCount();
  }
  for (auto i = 0; i < num_failures; ++i)
  {
    sm->incrementFailureCount();
  }

  sm->getCurrentIncrementalStatSnapshot(snapshot_out);
  ASSERT_EQ(snapshot_out.success_count, 90);
  ASSERT_EQ(snapshot_out.fail_count, 10);
  ASSERT_GT(snapshot_out.throughput, 90);
  ASSERT_NEAR(snapshot_out.quality, 0.9, 0.001);

  // Ensure counts reset after transaction
  sm->getCurrentIncrementalStatSnapshot(snapshot_out);
  ASSERT_EQ(snapshot_out.success_count, 0);
  ASSERT_EQ(snapshot_out.fail_count, 0);
  ASSERT_EQ(snapshot_out.throughput, 0);
  ASSERT_EQ(snapshot_out.quality, 0);

  // Next transaction
  for (auto i = 0; i < num_successes; ++i)
  {
    sm->incrementSuccessCount();
  }
  sm->getCurrentIncrementalStatSnapshot(snapshot_out);
  ASSERT_EQ(snapshot_out.success_count, 90);
  ASSERT_EQ(snapshot_out.fail_count, 0);
  ASSERT_GT(snapshot_out.throughput, 90);
  ASSERT_NEAR(snapshot_out.quality, 1.0, 0.001);

  ROS_INFO_STREAM("stats transaction durations counts");
}

TEST(Packml_CC, getCurrentStatTransaction_items)
{
  ROS_INFO_STREAM("CONTINUOUS CYCLE::stats transaction items");
  std::shared_ptr<AbstractStateMachine> sm = PackmlStateMachineContinuous::spawn();
  packml_sm::PackmlStatsSnapshot snapshot_out;

  sm->incrementQualityStatItem(1, 5, 15);
  sm->incrementErrorStatItem(2, 7, 30);

  sm->getCurrentIncrementalStatSnapshot(snapshot_out);
  ASSERT_EQ(snapshot_out.itemized_quality_map.size(), 1);
  ASSERT_EQ(snapshot_out.itemized_quality_map.at(1).count, 5);
  ASSERT_EQ(snapshot_out.itemized_quality_map.at(1).duration, 15);
  ASSERT_EQ(snapshot_out.itemized_error_map.size(), 1);
  ASSERT_EQ(snapshot_out.itemized_error_map.at(2).count, 7);
  ASSERT_EQ(snapshot_out.itemized_error_map.at(2).duration, 30);

  // Ensure map items' counts and durations reset after transaction
  sm->getCurrentIncrementalStatSnapshot(snapshot_out);
  ASSERT_EQ(snapshot_out.itemized_quality_map.size(), 1);
  ASSERT_EQ(snapshot_out.itemized_quality_map.at(1).count, 0);
  ASSERT_EQ(snapshot_out.itemized_quality_map.at(1).duration, 0);
  ASSERT_EQ(snapshot_out.itemized_error_map.size(), 1);
  ASSERT_EQ(snapshot_out.itemized_error_map.at(2).count, 0);
  ASSERT_EQ(snapshot_out.itemized_error_map.at(2).duration, 0);

  // Next transaction
  sm->incrementQualityStatItem(1, 88, 150);
  sm->getCurrentIncrementalStatSnapshot(snapshot_out);
  ASSERT_EQ(snapshot_out.itemized_quality_map.size(), 1);
  ASSERT_EQ(snapshot_out.itemized_quality_map.at(1).count, 88);
  ASSERT_EQ(snapshot_out.itemized_quality_map.at(1).duration, 150);

  ROS_INFO_STREAM("stats transaction durations items");
}

}
