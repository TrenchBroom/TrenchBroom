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

#include "FontDescriptor.h"

#include "kd/contracts.h"

#include <utility>

namespace tb::render
{
FontDescriptor::FontDescriptor(
  std::filesystem::path path,
  const size_t size,
  const unsigned char minChar,
  const unsigned char maxChar)
  : m_path{std::move(path)}
  , m_size{size}
  , m_minChar{minChar}
  , m_maxChar{maxChar}
{
  contract_pre(m_minChar <= m_maxChar);
}

const std::filesystem::path& FontDescriptor::path() const
{
  return m_path;
}

std::string FontDescriptor::name() const
{
  return m_path.stem().string();
}

size_t FontDescriptor::size() const
{
  return m_size;
}

unsigned char FontDescriptor::minChar() const
{
  return m_minChar;
}

unsigned char FontDescriptor::maxChar() const
{
  return m_maxChar;
}

unsigned char FontDescriptor::charCount() const
{
  return static_cast<unsigned char>(m_maxChar - m_minChar + 1);
}

} // namespace tb::render
