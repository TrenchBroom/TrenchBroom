/*
 Copyright (C) 2020 Kristian Duske

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

#include "RepeatStack.h"

#include "Ensure.h"

#include <kdl/set_temp.h>

#include <cassert>

namespace TrenchBroom
{
namespace View
{
RepeatStack::RepeatStack()
  : m_clearOnNextPush{false}
  , m_repeating{false}
{
}

size_t RepeatStack::size() const
{
  return m_stack.size();
}

void RepeatStack::push(RepeatableAction repeatableAction)
{
  if (!m_repeating)
  {
    if (m_openTransactionsStack.empty())
    {
      if (m_clearOnNextPush)
      {
        m_clearOnNextPush = false;
        clear();
      }

      m_stack.push_back(std::move(repeatableAction));
    }
    else
    {
      auto& openTransaction = m_openTransactionsStack.back();
      openTransaction.push_back(std::move(repeatableAction));
    }
  }
}

static void execute(const std::vector<RepeatStack::RepeatableAction>& actions)
{
  for (const auto& repeatable : actions)
  {
    repeatable();
  }
}

void RepeatStack::repeat() const
{
  if (!m_openTransactionsStack.empty())
  {
    return;
  }

  const auto repeating = kdl::set_temp{m_repeating};
  execute(m_stack);
}

void RepeatStack::clear()
{
  if (!m_openTransactionsStack.empty())
  {
    return;
  }
  assert(!m_repeating);
  m_stack.clear();
}

void RepeatStack::clearOnNextPush()
{
  if (!m_openTransactionsStack.empty())
  {
    return;
  }
  m_clearOnNextPush = true;
}

void RepeatStack::startTransaction()
{
  if (m_repeating)
  {
    return;
  }
  m_openTransactionsStack.emplace_back();
}

void RepeatStack::commitTransaction()
{
  if (m_repeating)
  {
    return;
  }
  ensure(!m_openTransactionsStack.empty(), "a transaction is open");

  auto transaction = std::move(m_openTransactionsStack.back());
  m_openTransactionsStack.pop_back();

  // discard empty transactions
  if (transaction.empty())
  {
    return;
  }

  // push it onto the next open transaction (or the main stack)
  push([transaction = std::move(transaction)]() { execute(transaction); });
}

void RepeatStack::rollbackTransaction()
{
  if (m_repeating)
  {
    return;
  }
  ensure(!m_openTransactionsStack.empty(), "a transaction is open");

  auto& openTransaction = m_openTransactionsStack.back();
  openTransaction.clear();
}
} // namespace View
} // namespace TrenchBroom
