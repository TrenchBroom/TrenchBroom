/*
 Copyright (C) 2021 Kristian Duske

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

#include "NotifierConnection.h"

#include "Notifier.h"

#include <kdl/memory_utils.h>

namespace TrenchBroom
{
NotifierConnection::NotifierConnection() = default;

NotifierConnection::NotifierConnection(
  std::weak_ptr<NotifierStateBase> notifier, const size_t id)
  : m_connections{{std::move(notifier), id}}
{
}

NotifierConnection::NotifierConnection(NotifierConnection&&) noexcept = default;

NotifierConnection& NotifierConnection::operator=(NotifierConnection&&) noexcept =
  default;

NotifierConnection::~NotifierConnection()
{
  disconnect();
}

NotifierConnection& NotifierConnection::operator+=(NotifierConnection&& other)
{
  m_connections.insert(
    std::end(m_connections),
    std::begin(other.m_connections),
    std::end(other.m_connections));
  other.m_connections.clear();
  return *this;
}

void NotifierConnection::disconnect()
{
  for (auto& [notifier, id] : m_connections)
  {
    if (!kdl::mem_expired(notifier))
    {
      kdl::mem_lock(notifier)->disconnect(id);
    }
  }
  m_connections.clear();
}
} // namespace TrenchBroom
