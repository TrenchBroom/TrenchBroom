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

#include "TestPreferenceManager.h"

namespace TrenchBroom
{
void TestPreferenceManager::initialize() {}

bool TestPreferenceManager::saveInstantly() const
{
  return false;
}

void TestPreferenceManager::saveChanges() {}

void TestPreferenceManager::discardChanges() {}

void TestPreferenceManager::validatePreference(PreferenceBase& preference)
{
  preference.setValid(true);
}

void TestPreferenceManager::savePreference(PreferenceBase&) {}
} // namespace TrenchBroom
