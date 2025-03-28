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

#pragma once

#include <filesystem>
#include <string>

namespace tb::render
{

class FontDescriptor
{
private:
  std::filesystem::path m_path;
  size_t m_size;
  unsigned char m_minChar;
  unsigned char m_maxChar;

public:
  FontDescriptor(
    std::filesystem::path path,
    size_t size,
    unsigned char minChar = ' ',
    unsigned char maxChar = '~');

  std::weak_ordering operator<=>(const FontDescriptor& other) const = default;
  bool operator==(const FontDescriptor& other) const = default;

  const std::filesystem::path& path() const;
  std::string name() const;
  size_t size() const;
  unsigned char minChar() const;
  unsigned char maxChar() const;
  unsigned char charCount() const;
};

} // namespace tb::render
