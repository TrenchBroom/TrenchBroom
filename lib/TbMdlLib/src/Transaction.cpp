/*
 Copyright (C) 2024 Kristian Duske

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

#include "mdl/Transaction.h"

#include "Logger.h"
#include "mdl/Map.h"
#include "mdl/TransactionScope.h"

#include "kd/contracts.h"

namespace tb::mdl
{

Transaction::Transaction(Map& map, std::string name)
  : m_map{map}
  , m_name{std::move(name)}
  , m_state{State::Running}
{
  begin();
}

Transaction::~Transaction()
{
  if (m_state == State::Running)
  {
    m_map.logger().error() << "Cancelling unfinished transaction with name '" << m_name
                           << "' - please report this on github!";
    cancel();
  }
}

Transaction::State Transaction::state() const
{
  return m_state;
}

void Transaction::finish(const bool commit)
{
  if (commit)
  {
    this->commit();
  }
  else
  {
    cancel();
  }
}

bool Transaction::commit()
{
  contract_pre(m_state == State::Running);

  if (m_map.commitTransaction())
  {
    m_state = State::Committed;
    return true;
  }

  m_state = State::Cancelled;
  return false;
}

void Transaction::rollback()
{
  contract_pre(m_state == State::Running);

  m_map.rollbackTransaction();
}

void Transaction::cancel()
{
  contract_pre(m_state == State::Running);

  m_map.cancelTransaction();
  m_state = State::Cancelled;
}

void Transaction::begin()
{
  m_map.startTransaction(m_name, TransactionScope::Oneshot);
}

} // namespace tb::mdl
