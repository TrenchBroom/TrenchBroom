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

#include "TestPreferenceStore.h"

namespace tb
{

bool TestPreferenceStore::load(const std::filesystem::path&, bool&)
{
  return false;
}

bool TestPreferenceStore::load(const std::filesystem::path&, int&)
{
  return false;
}

bool TestPreferenceStore::load(const std::filesystem::path&, float&)
{
  return false;
}

bool TestPreferenceStore::load(const std::filesystem::path&, std::string&)
{
  return false;
}

bool TestPreferenceStore::load(const std::filesystem::path&, std::filesystem::path&)
{
  return false;
}

bool TestPreferenceStore::load(const std::filesystem::path&, Color&)
{
  return false;
}

bool TestPreferenceStore::load(const std::filesystem::path&, QKeySequence&)
{
  return false;
}

void TestPreferenceStore::save(const std::filesystem::path&, bool) {}

void TestPreferenceStore::save(const std::filesystem::path&, int) {}

void TestPreferenceStore::save(const std::filesystem::path&, float) {}

void TestPreferenceStore::save(const std::filesystem::path&, const std::string&) {}

void TestPreferenceStore::save(const std::filesystem::path&, const std::filesystem::path&)
{
}

void TestPreferenceStore::save(const std::filesystem::path&, const Color&) {}

void TestPreferenceStore::save(const std::filesystem::path&, const QKeySequence&) {}


} // namespace tb
