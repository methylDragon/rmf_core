/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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

#include "utils_Trajectory.hpp"

#include <src/rmf_traffic/debug_Trajectory.hpp>

#include <rmf_utils/catch.hpp>

#include <iostream>

SCENARIO("Class Profile unit tests")
{

  GIVEN("Checking Accessor Functions")
  {
    std::shared_ptr<rmf_traffic::geometry::Box> profile_shape =
        std::make_shared<rmf_traffic::geometry::Box>(1.0, 1.0);
    rmf_traffic::Trajectory::ProfilePtr profile =
        rmf_traffic::Trajectory::Profile::make_strict(profile_shape);

    WHEN("Initial Configuration")
    {
      REQUIRE(profile->get_agency() ==
              rmf_traffic::Trajectory::Profile::Agency::Strict);
      REQUIRE(profile->get_shape() == profile_shape);
    }

    WHEN("Change Agency to Autonomous")
    {
      profile->set_to_autonomous();
      CHECK(profile->get_agency() ==
            rmf_traffic::Trajectory::Profile::Agency::Autonomous);
    }

    WHEN("Change Agency to Queued")
    {
      //TODO: Should QueueID be string?
      const std::string queue_id = "1";
      profile->set_to_queued(queue_id);
      CHECK(profile->get_queue_info()->get_queue_id() == queue_id);
    }

    WHEN("Change Shape to Unit Circle")
    {
      std::shared_ptr<rmf_traffic::geometry::Circle> new_profile_shape =
          std::make_shared<rmf_traffic::geometry::Circle>(1.0);
      profile->set_shape(new_profile_shape);

      CHECK(profile->get_agency() ==
            rmf_traffic::Trajectory::Profile::Agency::Strict);
      CHECK(profile->get_shape() == new_profile_shape);
    }
  }

  GIVEN("Checking Dirty Input")
  {
    rmf_traffic::Trajectory::ProfilePtr profile = make_test_profile(UnitBox);

    WHEN("Giving nullptr as queue_id is not allowed")
    {
      CHECK_THROWS(profile->set_to_queued(nullptr));
    }
  }
}

SCENARIO("base_iterator unit tests")
{
  GIVEN("Testing operators")
  {
    using namespace std::chrono_literals;

    rmf_traffic::Trajectory trajectory{"test_map"};
    REQUIRE(trajectory.begin() == trajectory.end());
    REQUIRE(trajectory.end() == trajectory.end());

    const auto finish_time = std::chrono::steady_clock::now();
    const auto profile = make_test_profile(UnitBox);
    const Eigen::Vector3d begin_pos = Eigen::Vector3d(1, 1, 1);
    const Eigen::Vector3d begin_vel = Eigen::Vector3d(1, 1, 1);

    auto result = trajectory.insert(finish_time, profile, begin_pos, begin_vel);
    rmf_traffic::Trajectory::iterator first_it = result.it;
    REQUIRE(result.inserted);
    REQUIRE(trajectory.begin() == first_it);

    const auto finish_time_2 = std::chrono::steady_clock::now() + 10s;
    const auto profile_2 = make_test_profile(UnitBox);
    const Eigen::Vector3d begin_pos_2 = Eigen::Vector3d(2, 0, 3);
    const Eigen::Vector3d begin_vel_2 = Eigen::Vector3d(2, 0, 3);

    auto result_2 = trajectory.insert(finish_time_2, profile_2, begin_pos_2, begin_vel_2);
    rmf_traffic::Trajectory::iterator second_it = result_2.it;
    REQUIRE(result_2.inserted);
    REQUIRE(--trajectory.end() == second_it); // trajectory.end() is a placeholder "beyond" the last element

    WHEN("Comparison operators")
    {
      CHECK((*first_it).get_profile() == profile);
      CHECK(first_it->get_profile() == profile);
      CHECK(first_it == trajectory.begin());
      CHECK(trajectory.begin() != trajectory.end());
      CHECK(first_it != trajectory.end());
      CHECK(first_it < trajectory.end());
      CHECK(first_it <= trajectory.end());
      CHECK(trajectory.end() > first_it);
      CHECK(trajectory.end() >= trajectory.end());
    }

    WHEN("Mutating iterators")
    {
      first_it++;
      CHECK(first_it == second_it);
      first_it--;
      CHECK(first_it != second_it);
      CHECK(first_it < second_it);
    }
  }
}

SCENARIO("Class Segment unit tests")
{
  GIVEN("Testing accessor functions")
  {
    using namespace std::chrono_literals;

    rmf_traffic::Trajectory trajectory{"test_map"};
    REQUIRE(trajectory.begin() == trajectory.end());
    REQUIRE(trajectory.end() == trajectory.end());

    const auto finish_time = std::chrono::steady_clock::now();
    const auto profile = make_test_profile(UnitBox);
    const Eigen::Vector3d begin_pos = Eigen::Vector3d(0, 0, 0);
    const Eigen::Vector3d begin_vel = Eigen::Vector3d(0, 0, 0);

    auto result = trajectory.insert(finish_time, profile, begin_pos, begin_vel);
    REQUIRE(result.inserted);

    rmf_traffic::Trajectory::Segment segment = *trajectory.find(finish_time);

    WHEN("Initial Configuration")
    {
      REQUIRE(segment.get_profile() == profile);
      REQUIRE(segment.get_finish_position() == begin_pos);
      REQUIRE(segment.get_finish_velocity() == begin_vel);
      REQUIRE(segment.get_finish_time() == finish_time);
    }

    WHEN("Setting a new profile")
    {
      const auto new_profile = make_test_profile(UnitCircle);
      segment.set_profile(new_profile);
      CHECK(segment.get_profile() == new_profile);
      CHECK(segment.get_profile() != profile);
    }

    WHEN("Mutating current profile")
    {
      profile->set_to_autonomous();
      CHECK(segment.get_profile()->get_agency() ==
            rmf_traffic::Trajectory::Profile::Agency::Autonomous);

      const auto new_shape = std::make_shared<rmf_traffic::geometry::Circle>(1.0);
      profile->set_shape(new_shape);
      CHECK(segment.get_profile()->get_shape() == new_shape);
    }

    // TODO: The Docs record this as a 2D homogenous position, should be 3D
    WHEN("Setting a new position")
    {
      const Eigen::Vector3d new_pos = Eigen::Vector3d(1, 1, 1);
      segment.set_finish_position(new_pos);
      CHECK(segment.get_finish_position() == new_pos);
      CHECK(segment.get_finish_position() != begin_pos);
    }

    // TODO: The Docs record this as a 2D homogenous position, should be 3D
    WHEN("Setting a new velocity")
    {
      const Eigen::Vector3d new_vel = Eigen::Vector3d(1, 1, 1);
      segment.set_finish_velocity(new_vel);
      CHECK(segment.get_finish_velocity() == new_vel);
      CHECK(segment.get_finish_velocity() != begin_vel);
    }

    WHEN("Setting a finish time")
    {
      const auto new_finish_time = std::chrono::steady_clock::now() + 15s;
      segment.set_finish_time(new_finish_time);
      CHECK(segment.get_finish_time() == new_finish_time);
      CHECK(segment.get_finish_time() != finish_time);
    }
  }

  GIVEN("Test automatic reordering when setting finish times")
  {
    using namespace std::chrono_literals;

    rmf_traffic::Trajectory trajectory{"test_map"};
    REQUIRE(trajectory.begin() == trajectory.end());
    REQUIRE(trajectory.end() == trajectory.end());

    const auto finish_time = std::chrono::steady_clock::now();
    const auto profile = make_test_profile(UnitBox);
    const Eigen::Vector3d begin_pos = Eigen::Vector3d(1, 1, 1);
    const Eigen::Vector3d begin_vel = Eigen::Vector3d(1, 1, 1);

    auto result = trajectory.insert(finish_time, profile, begin_pos, begin_vel);
    const rmf_traffic::Trajectory::iterator first_it = result.it;
    REQUIRE(result.inserted);

    const auto finish_time_2 = std::chrono::steady_clock::now() + 10s;
    const auto profile_2 = make_test_profile(UnitBox);
    const Eigen::Vector3d begin_pos_2 = Eigen::Vector3d(2, 0, 3);
    const Eigen::Vector3d begin_vel_2 = Eigen::Vector3d(2, 0, 3);

    auto result_2 = trajectory.insert(finish_time_2, profile_2, begin_pos_2, begin_vel_2);
    const rmf_traffic::Trajectory::iterator second_it = result_2.it;
    REQUIRE(result_2.inserted);

    const auto finish_time_3 = std::chrono::steady_clock::now() + 20s;
    const auto profile_3 = make_test_profile(UnitBox);
    const Eigen::Vector3d begin_pos_3 = Eigen::Vector3d(4, 2, 6);
    const Eigen::Vector3d begin_vel_3 = Eigen::Vector3d(6, 2, 4);

    auto result_3 = trajectory.insert(finish_time_3, profile_3, begin_pos_3, begin_vel_3);
    const rmf_traffic::Trajectory::iterator third_it = result_3.it;
    REQUIRE(result_3.inserted);

    REQUIRE(trajectory.begin() == first_it);
    REQUIRE(first_it < second_it);
    REQUIRE(second_it < third_it);

    WHEN("Single forward time shift for one positional swap")
    {
      const auto new_finish_time = finish_time + 15s;
      first_it->set_finish_time(new_finish_time);
      CHECK(second_it < first_it);
      CHECK(first_it < third_it);
    }

    WHEN("Single forward time shift for two positional swap")
    {
      const auto new_finish_time = finish_time + 25s;
      first_it->set_finish_time(new_finish_time);
      CHECK(second_it < third_it);
      CHECK(third_it < first_it);
    }

    WHEN("Single backward time shift for one positional swap")
    {
      const auto new_finish_time = finish_time_3 - 15s;
      third_it->set_finish_time(new_finish_time);
      CHECK(first_it < third_it);
      CHECK(third_it < second_it);
    }

     WHEN("Single backward time shift for two positional swap")
    {
      const auto new_finish_time = finish_time_3 - 25s;
      third_it->set_finish_time(new_finish_time);

      CHECK(rmf_traffic::Trajectory::Debug::check_iterator_time_consistency(
            trajectory, true));

      CHECK(third_it < first_it);
      CHECK(first_it < second_it);
    }

    WHEN("Forward time shift with time conflict")
    {
      CHECK_THROWS(first_it->set_finish_time(finish_time_2));
    }

    WHEN("Backward time shift with time conflict")
    {
      CHECK_THROWS(third_it->set_finish_time(finish_time_2));
    }

    WHEN("Adding times across all segments") 
    {
      first_it->adjust_finish_times(2s);

      CHECK(rmf_traffic::Trajectory::Debug::check_iterator_time_consistency(
            trajectory, true));

      CHECK(first_it->get_finish_time() == finish_time + 2s);
      CHECK(second_it->get_finish_time() == finish_time_2 + 2s);
      CHECK(third_it->get_finish_time() == finish_time_3 + 2s);
    }

  }

}

// TEST_CASE("Construct a Trajectory")
// {
//   using namespace std::chrono_literals;

//   rmf_traffic::Trajectory trajectory{"test_map"};
//   CHECK(trajectory.begin() == trajectory.end());
//   CHECK(trajectory.end() == trajectory.end());

//   const auto profile = make_test_profile();

//   const auto finish_time = std::chrono::steady_clock::now();
//   const Eigen::Vector3d begin_p = Eigen::Vector3d(0, 0, 0);
//   const Eigen::Vector3d begin_v = Eigen::Vector3d(0, 0, 0);

//   auto result = trajectory.insert(
//         finish_time, profile, begin_p, begin_v);

//   const rmf_traffic::Trajectory::iterator first_it = result.it;
//   CHECK(result.inserted);

//   CHECK(first_it == trajectory.begin());
//   CHECK(trajectory.begin() != trajectory.end());
//   CHECK(first_it != trajectory.end());
//   CHECK(first_it < trajectory.end());
//   CHECK(first_it <= trajectory.end());
//   CHECK(trajectory.end() > first_it);
//   CHECK(trajectory.end() >= trajectory.end());

//   CHECK(begin_p == first_it->get_finish_position());
//   CHECK(begin_v == first_it->get_finish_velocity());
//   CHECK(finish_time == first_it->get_finish_time());

//   const auto second_time = finish_time + 10s;
//   const Eigen::Vector3d second_p = Eigen::Vector3d(1, 2, 3);
//   const Eigen::Vector3d second_v = Eigen::Vector3d(3, 2, 1);

//   result = trajectory.insert(
//         second_time, profile, second_p, second_v);

//   const rmf_traffic::Trajectory::iterator second_it = result.it;
//   CHECK(result.inserted);

//   CHECK(second_it == ++trajectory.begin());
//   CHECK(second_it != trajectory.begin());
//   CHECK(second_it > trajectory.begin());
//   CHECK(second_it >= trajectory.begin());
//   CHECK(trajectory.begin() < second_it);
//   CHECK(trajectory.begin() <= second_it);

//   CHECK(second_it != first_it);
//   CHECK(second_it > first_it);
//   CHECK(second_it >= first_it);
//   CHECK(first_it < second_it);
//   CHECK(first_it <= second_it);

//   CHECK(second_it != trajectory.end());
//   CHECK(second_it < trajectory.end());
//   CHECK(second_it <= trajectory.end());
//   CHECK(trajectory.end() > second_it);
//   CHECK(trajectory.end() >= second_it);

//   CHECK(second_it->get_finish_position() == second_p);
//   CHECK(second_it->get_finish_velocity() == second_v);
//   CHECK(second_it->get_finish_time() == second_time);
// }

// TEST_CASE("Copy and move a trajectory")
// {
//   using namespace std::chrono_literals;

//   rmf_traffic::Trajectory trajectory{"test_map"};

//   const auto finish_time = std::chrono::steady_clock::now();

//   trajectory.insert(
//         finish_time, make_test_profile(),
//         Eigen::Vector3d::UnitX(),
//         Eigen::Vector3d::UnitX());

//   trajectory.insert(
//         finish_time + 10s, make_test_profile(),
//         Eigen::Vector3d::UnitY(),
//         Eigen::Vector3d::UnitY());

//   trajectory.insert(
//         finish_time + 15s, make_test_profile(),
//         Eigen::Vector3d::UnitZ(),
//         Eigen::Vector3d::UnitZ());

//   rmf_traffic::Trajectory copy = trajectory;

//   rmf_traffic::Trajectory::const_iterator ot = trajectory.begin();
//   rmf_traffic::Trajectory::const_iterator ct = copy.begin();
//   for( ; ot != trajectory.end() && ct != trajectory.end(); ++ot, ++ct)
//   {
//     CHECK(ot->get_profile() == ct->get_profile());
//     CHECK(ot->get_finish_position() == ct->get_finish_position());
//     CHECK(ot->get_finish_velocity() == ct->get_finish_velocity());
//     CHECK(ot->get_finish_time() == ct->get_finish_time());
//   }
//   CHECK(ot == trajectory.end());
//   CHECK(ct == copy.end());

//   for(auto it = copy.begin(); it != copy.end(); ++it)
//   {
//     it->set_profile(make_test_profile());
//     it->set_finish_position(it->get_finish_position() + Eigen::Vector3d::UnitZ());
//     it->set_finish_velocity(it->get_finish_velocity() + Eigen::Vector3d::UnitZ());
//     it->set_finish_time(it->get_finish_time() + 2s);
//   }

//   ot = trajectory.begin();
//   ct = copy.begin();
//   for( ; ot != trajectory.end() && ct != trajectory.end(); ++ot, ++ct)
//   {
//     CHECK(ot->get_profile() != ct->get_profile());
//     CHECK(ot->get_finish_position() != ct->get_finish_position());
//     CHECK(ot->get_finish_velocity() != ct->get_finish_velocity());
//     CHECK(ot->get_finish_time() != ct->get_finish_time());
//   }
//   CHECK(ot == trajectory.end());
//   CHECK(ct == copy.end());

//   // Copy again
//   copy = trajectory;

//   // Now move the original
//   rmf_traffic::Trajectory moved = std::move(trajectory);

//   ct = copy.begin();
//   rmf_traffic::Trajectory::const_iterator mt = moved.begin();
//   for( ; ct != copy.end() && mt != moved.end(); ++ct, ++mt)
//   {
//     CHECK(ct->get_profile() == mt->get_profile());
//     CHECK(ct->get_finish_position() == mt->get_finish_position());
//     CHECK(ct->get_finish_velocity() == mt->get_finish_velocity());
//     CHECK(ct->get_finish_time() == mt->get_finish_time());
//   }
//   CHECK(ct == copy.end());
//   CHECK(mt == moved.end());
// }