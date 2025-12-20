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

#include "PreferenceStore.h"

namespace tb
{

class TestPreferenceStore : public PreferenceStore
{
public:
  bool load(const std::filesystem::path& path, bool& value) override;
  bool load(const std::filesystem::path& path, int& value) override;
  bool load(const std::filesystem::path& path, float& value) override;
  bool load(const std::filesystem::path& path, std::string& value) override;
  bool load(const std::filesystem::path& path, std::filesystem::path& value) override;
  bool load(const std::filesystem::path& path, Color& value) override;
  bool load(const std::filesystem::path& path, QKeySequence& value) override;

  void save(const std::filesystem::path& path, bool value) override;
  void save(const std::filesystem::path& path, int value) override;
  void save(const std::filesystem::path& path, float value) override;
  void save(const std::filesystem::path& path, const std::string& value) override;
  void save(
    const std::filesystem::path& path, const std::filesystem::path& value) override;
  void save(const std::filesystem::path& path, const Color& value) override;
  void save(const std::filesystem::path& path, const QKeySequence& value) override;
};

} // namespace tb
