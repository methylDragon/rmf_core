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

#ifndef SRC__RMF_FLEET_ADAPTER__PHASES__RXOPERATORS_HPP
#define SRC__RMF_FLEET_ADAPTER__PHASES__RXOPERATORS_HPP

#include <rxcpp/rx.hpp>

namespace rmf_fleet_adapter {
namespace phases {

/**
 * Similar to `take_while`, but also emits the item that fails the predicate.
 * @tparam T
 * @tparam Predicate
 * @param pred
 * @return
 */
template<typename Predicate>
auto grab_while(Predicate pred)
{
  return [pred = std::move(pred)](const auto& s)
  {
    using SourceType = typename std::decay_t<decltype(s)>::value_type;
    rxcpp::composite_subscription subscription{};
    return rxcpp::make_subscriber<SourceType>(
      subscription,
      [=, pred = std::move(pred)](const SourceType& v)
      {
        s.on_next(v);
        if (!pred(v))
        {
          s.on_completed();
          subscription.unsubscribe();
        }
      });
  };
}

/**
 * Runs a function when a subscribe happens.
 * @tparam T
 * @tparam F
 * @param f
 * @return
 */
template<typename F>
auto on_subscribe(F f)
{
  return [f = std::move(f)](const auto& s)
  {
    f();
    return s;
  };
}

} // namespace phases
} // namespace rmf_fleet_adapter

#endif // SRC__RMF_FLEET_ADAPTER__PHASES__RXOPERATORS_HPP