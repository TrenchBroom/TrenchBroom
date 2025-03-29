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

#include "AttrString.h"

#include "Macros.h"

#include "kdl/zip_iterator.h"

#include <compare>

namespace tb::render
{

AttrString::LineFunc::~LineFunc() = default;

void AttrString::LineFunc::process(const std::string& str, const Justify justify)
{
  switch (justify)
  {
  case Justify::Left:
    justifyLeft(str);
    break;
  case Justify::Right:
    justifyRight(str);
    break;
  case Justify::Center:
    center(str);
    break;
    switchDefault();
  }
}

AttrString::AttrString() = default;

AttrString::AttrString(const std::string& string)
{
  appendLeftJustified(string);
}

std::strong_ordering AttrString::operator<=>(const AttrString& other) const
{
  // AppleClang 15 doesn't provide operator<=> for std::vector, nor
  // std::lexicographical_compare_three_way, so we have to do it manually
  if (const auto cmp = m_lines.size() <=> other.m_lines.size(); cmp != 0)
  {
    return cmp;
  }

  for (const auto& [mine, theirs] : kdl::make_zip_range(m_lines, other.m_lines))
  {
    if (const auto cmp = mine <=> theirs; cmp != 0)
    {
      return cmp;
    }
  }
  return std::strong_ordering::equal;
}

bool AttrString::operator==(const AttrString& other) const
{
  return *this <=> other == 0;
}

void AttrString::lines(LineFunc& func) const
{
  for (size_t i = 0; i < m_lines.size(); ++i)
  {
    func.process(m_lines[i].string, m_lines[i].justify);
  }
}

void AttrString::appendLeftJustified(std::string string)
{
  m_lines.push_back(Line{std::move(string), Justify::Left});
}

void AttrString::appendRightJustified(std::string string)
{
  m_lines.push_back(Line{std::move(string), Justify::Right});
}

void AttrString::appendCentered(std::string string)
{
  m_lines.push_back(Line{std::move(string), Justify::Center});
}

} // namespace tb::render
