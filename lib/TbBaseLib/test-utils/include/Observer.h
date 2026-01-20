/*
 Copyright (C) 2025 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "Notifier.h"
#include "NotifierConnection.h"

#include <vector>

namespace tb
{
template <typename... T>
struct Observer
{
  NotifierConnection connection;
  std::vector<std::tuple<T...>> notifications;

  template <typename... X>
  explicit Observer(Notifier<X...>& notifier)
  {
    static_assert(sizeof...(X) == sizeof...(T));
    static_assert((std::is_convertible_v<X, T> && ...));

    connection += notifier.connect([&]<typename... Y>(Y&&... x) {
      static_assert(sizeof...(Y) == sizeof...(T));
      static_assert((std::is_convertible_v<Y, T> && ...));

      notifications.emplace_back(std::forward<Y>(x)...);
    });
  }

  void reset() { notifications.clear(); }
};


} // namespace tb
