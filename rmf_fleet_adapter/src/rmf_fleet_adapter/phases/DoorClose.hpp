/*
 * Copyright (C) 2020 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifndef SRC__RMF_FLEET_ADAPTER__PHASES__DOORCLOSE_HPP
#define SRC__RMF_FLEET_ADAPTER__PHASES__DOORCLOSE_HPP

#include "DoorControlAction.hpp"
#include "../Task.hpp"

#include <rmf_rxcpp/Transport.hpp>

namespace rmf_fleet_adapter {
namespace phases {

struct DoorClose
{
  class ActivePhase : public Task::ActivePhase
  {
  public:

    ActivePhase(
      std::string door_name,
      std::shared_ptr<rmf_rxcpp::Transport> transport,
      rxcpp::observable<rmf_door_msgs::msg::DoorState> door_state_obs,
      rxcpp::observable<rmf_door_msgs::msg::SupervisorHeartbeat> supervisor_heartbeat_obs);

    const rxcpp::observable<Task::StatusMsg>& observe() const override;

    rmf_traffic::Duration estimate_remaining_time() const override;

    void emergency_alarm(bool on) override;

    void cancel() override;

    const std::string& description() const override;

  private:

    std::string _door_name;
    std::shared_ptr<rmf_rxcpp::Transport> _transport;
    rxcpp::observable<Task::StatusMsg> _job;
    rxcpp::observable<rmf_door_msgs::msg::DoorState> _door_state_obs;
    rxcpp::observable<rmf_door_msgs::msg::SupervisorHeartbeat> _supervisor_heartbeat_obs;
    std::string _description;
  };

  class PendingPhase : public Task::PendingPhase
  {
  public:

    PendingPhase(
      std::string door_name,
      std::shared_ptr<rmf_rxcpp::Transport> transport,
      rxcpp::observable<rmf_door_msgs::msg::DoorState> door_state_obs,
      rxcpp::observable<rmf_door_msgs::msg::SupervisorHeartbeat> supervisor_heartbeat_obs);

    std::shared_ptr<Task::ActivePhase> begin() override;

    rmf_traffic::Duration estimate_phase_duration() const override;

    const std::string& description() const override;

  private:

    std::string _door_name;
    std::shared_ptr<rmf_rxcpp::Transport> _transport;
    rxcpp::observable<rmf_door_msgs::msg::DoorState> _door_state_obs;
    rxcpp::observable<rmf_door_msgs::msg::SupervisorHeartbeat> _supervisor_heartbeat_obs;
    std::string _description;
  };
};

} // namespace phases
} // namespace rmf_fleet_adapter

#endif // SRC__RMF_FLEET_ADAPTER__PHASES__DOORCLOSE_HPP