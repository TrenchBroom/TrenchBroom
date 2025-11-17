/*
 Copyright (C) 2010 Kristian Duske

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

#include "kd/dynamic_bitset.h"

namespace kdl
{

dynamic_bitset::dynamic_bitset(const std::size_t initialSize)
  : m_bits(initialSize, false)
{
}

bool dynamic_bitset::operator[](const std::size_t index) const
{
  return index < m_bits.size() ? m_bits[index] : false;
}

std::vector<bool>::reference dynamic_bitset::dynamic_bitset::operator[](
  const std::size_t index)
{
  if (index >= m_bits.size())
  {
    m_bits.insert(std::end(m_bits), index - m_bits.size() + 1, false);
  }
  return m_bits[index];
}

void dynamic_bitset::reset()
{
  m_bits = std::vector<bool>(64, false);
}

} // namespace kdl
