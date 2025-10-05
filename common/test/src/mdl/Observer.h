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

#include <set>

namespace tb::mdl
{
template <typename C>
struct Observer
{
  std::set<C> collected;

  void operator()(C c) { collected.insert(std::forward<C>(c)); }
};

template <>
struct Observer<void>
{
  NotifierConnection connection;
  bool called = false;

  void operator()() { called = true; }

  explicit Observer(Notifier<>& notifier)
  {
    connection += notifier.connect(this, &Observer::operator());
  }
};

template <typename C>
struct Observer<C*>
{
  NotifierConnection connection;
  std::set<C*> collected;

  void operator()(C* c) { collected.insert(c); }

  explicit Observer(Notifier<C*>& notifier)
  {
    connection += notifier.connect(this, &Observer::operator());
  }
};

template <typename C>
struct Observer<C&>
{
  NotifierConnection connection;
  std::set<C*> collected;

  void operator()(C& c) { collected.insert(&c); }

  explicit Observer(Notifier<C&>& notifier)
  {
    connection += notifier.connect(this, &Observer::operator());
  }
};

template <typename T, template <typename...> class Collection>
struct Observer<Collection<T>>
{
  NotifierConnection connection;
  std::set<T> collected;

  void operator()(const Collection<T>& collection)
  {
    collected.insert(collection.begin(), collection.end());
  }

  explicit Observer(Notifier<const Collection<T>&>& notifier)
  {
    connection += notifier.connect(this, &Observer::operator());
  }
};


} // namespace tb::mdl
