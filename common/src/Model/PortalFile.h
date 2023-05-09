/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include <vecmath/forward.h>

#include <filesystem>
#include <vector>

namespace TrenchBroom
{
namespace Model
{
class PortalFile
{
private:
  std::vector<vm::polygon3f> m_portals;

public:
  PortalFile();
  ~PortalFile();

  /**
   * Constructor throws an exception if portalFilePath couldn't be read.
   */
  explicit PortalFile(const std::filesystem::path& path);

  static bool canLoad(const std::filesystem::path& path);

  const std::vector<vm::polygon3f>& portals() const;

private:
  void load(const std::filesystem::path& path);
};
} // namespace Model
} // namespace TrenchBroom
